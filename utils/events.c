#include "utils.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static void build_avl_packet(
    unsigned char *pkt,
    long long ts,
    int32_t lon,
    int32_t lat,
    uint16_t speed_kmh
) {
    static const unsigned char base[AVL_PACKET_SIZE] = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x21,
        0x08,
        0x01,
        0x00,0x00,0x01,0x94,0x4A,0xC0,0x00,0x00,
        0x01,
        0xF8,0xA4,0xA1,0xB0,
        0xFF,0x72,0x8B,0x50,
        0x00,0xB4,
        0x01,0x2C,
        0x00,
        0x00,0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x01,
        0x00,0x00,0x00,0x00
    };

    memcpy(pkt, base, AVL_PACKET_SIZE);

    uint32_t avl_len = 33;
    uint32_t net_len = htonl(avl_len);
    memcpy(&pkt[4], &net_len, 4);

    write_int64(&pkt[10], ts);
    write_int32(&pkt[19], lon);
    write_int32(&pkt[23], lat);

    pkt[32] = (speed_kmh >> 8) & 0xFF;
    pkt[33] = speed_kmh & 0xFF;

    uint32_t crc = crc16_teltonika(&pkt[8], avl_len);
    write_crc(&pkt[AVL_PACKET_SIZE - 4], crc);
}

int send_all(int sock, unsigned char *buf, int len) {
    int total = 0;

    while (total < len) {
        int n = send(sock, buf + total, len - total, 0);

        if (n <= 0)
            return n;

        total += n;
    }

    return total;
}

int read_exact(int sock, unsigned char *buf, int len) {
    int total = 0;

    while (total < len) {
        int n = recv(sock, buf + total, len - total, 0);
        if (n <= 0)
            return n;
        total += n;
    }

    return total;
}

static int read_ack(int device_id, int sock, unsigned char buffer[BUFFER_SIZE]) {
    int n = read_exact(sock, buffer, 4);

    if (n != 4) {
        printf("server did NOT acknowledge\n");
        return -1;
    }

    uint32_t ack;
    memcpy(&ack, buffer, 4);
    ack = ntohl(ack);

    printf("[device %d] Server acknowledged: %u\n", device_id, ack);
    return ack;
}

void simulate_movement(
    int device_id,
    int *sock,
    unsigned char avl_packet[AVL_PACKET_SIZE],
    unsigned char buffer[BUFFER_SIZE],
    double (*route)[2],
    size_t route_len
) {
    printf("\nSIMULATING MOVEMENT\n");

    for (size_t i = 0; i < route_len; i++) {
        printf("[device %d] sending lat=%.6f lon=%.6f\n",
               device_id, route[i][0], route[i][1]);

        long long ts = current_time_ms();

        int32_t lat = (int32_t)(route[i][0] * 10000000);
        int32_t lon = (int32_t)(route[i][1] * 10000000);

        size_t prev = (i > 0) ? i - 1 : i;

        double dlat = (route[i][0] - route[prev][0]) * 111000.0;
        double dlon = (route[i][1] - route[prev][1]) *
                      111000.0 *
                      cos(route[i][0] * M_PI / 180.0);

        double dist_m = sqrt(dlat * dlat + dlon * dlon);
        uint16_t speed_kmh = (uint16_t)((dist_m / 3.0) * 3.6);

        build_avl_packet(avl_packet, ts, lon, lat, speed_kmh);

        if (send_all(*sock, avl_packet, AVL_PACKET_SIZE) != AVL_PACKET_SIZE) {
            printf("[device %d] send failed\n", device_id);
            return;
        }

        if (read_ack(device_id, *sock, buffer) < 0)
            return;

        sleep(3);
    }
}
void simulate_stop(
    int device_id,
    int *sock,
    unsigned char avl_packet[AVL_PACKET_SIZE],
    unsigned char buffer[BUFFER_SIZE],
    double stop_lat,
    double stop_lon
) {
    printf("\nSIMULATING GEOFENCE STOP\n");

    int32_t slat = (int32_t)(stop_lat * 10000000);
    int32_t slon = (int32_t)(stop_lon * 10000000);

    for (int i = 0; i < 3; i++) {
        long long ts = current_time_ms();

        build_avl_packet(avl_packet, ts, slon, slat, 0);

        if (send_all(*sock, avl_packet, AVL_PACKET_SIZE) != AVL_PACKET_SIZE) {
            printf("[device %d] send failed\n", device_id);
            return;
        }

        int ack = read_ack(device_id, *sock, buffer);
        printf("Stop packet %d sent (t=%lld ms), ACK=%d\n", i, ts, ack);

        sleep(2);
    }

    printf("\nRESETTING - sending movement packet\n");

    long long ts = current_time_ms();
    build_avl_packet(avl_packet, ts, slon, slat, 10);

    if (send_all(*sock, avl_packet, AVL_PACKET_SIZE) != AVL_PACKET_SIZE) {
        printf("[device %d] send failed\n", device_id);
        return;
    }

    read_ack(device_id, *sock, buffer);
    printf("Movement reset packet sent\n");
}

