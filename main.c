#include <pthread.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "utils/utils.h"

#define SERVER_IP "::1"
#define SERVER_PORT 5027
double route1[][2] = {
  {-8.12980, -79.02320},
  {-8.11010, -79.02390},
  {-8.11040, -79.02360},
  {-8.11070, -79.02330},
  {-8.11100, -79.02300},
  {-8.11130, -79.02270},
  {-8.11150, -79.02240},
  {-8.11155, -79.02210},
  {-8.11160, -79.02190},
  {-8.11145, -79.02160},
  {-8.11120, -79.02140},
  {-8.11090, -79.02120},
  {-8.11060, -79.02100},
    // transition toward the polygon
{-8.11080, -79.02600},
{-8.11100, -79.02750},
{-8.11130, -79.02850},
{-8.11160, -79.02920},

// --- entering the POLYGON ---
// POLYGON ((-8.111963 -79.030297,
//           -8.111866 -79.030049,
//           -8.112134 -79.029954,
//           -8.112219 -79.030221,
//           -8.111963 -79.030297))
{-8.11196, -79.03010},   // near entry edge
{-8.11200, -79.03005},   // inside
{-8.11213, -79.02998},   // deeper inside
{-8.11220, -79.03015},   // crossing toward exit edge
{-8.11210, -79.03025},   // near exit edge
// --- exiting the POLYGON ---

// continue past
{-8.11200, -79.03060},
{-8.11180, -79.03100},
{-8.11160, -79.03150}

};
double route2[][2] = {
  {-8.13500, -79.03100},
  {-8.13200, -79.02900},
  {-8.12900, -79.02700},
  {-8.11070, -79.02330},
  {-8.11100, -79.02300},
  {-8.11130, -79.02270},
  {-8.11150, -79.02240},
  {-8.11155, -79.02210},
  {-8.11160, -79.02190},
  {-8.11145, -79.02160},
  {-8.11120, -79.02140},
  {-8.11090, -79.02120},
  {-8.11060, -79.02100},
    // transition toward the polygon
{-8.11080, -79.02600},
{-8.11100, -79.02750},
{-8.11130, -79.02850},
{-8.11160, -79.02920},

// --- entering the POLYGON ---
// POLYGON ((-8.111963 -79.030297,
//           -8.111866 -79.030049,
//           -8.112134 -79.029954,
//           -8.112219 -79.030221,
//           -8.111963 -79.030297))
{-8.11196, -79.03010},   // near entry edge
{-8.11200, -79.03005},   // inside
{-8.11213, -79.02998},   // deeper inside
{-8.11220, -79.03015},   // crossing toward exit edge
{-8.11210, -79.03025},   // near exit edge
// --- exiting the POLYGON ---

// continue past
{-8.11200, -79.03060},
{-8.11180, -79.03100},
{-8.11160, -79.03150}

};

typedef struct {
  const char *imei;
  int        device_id;
  double     (*route)[2];
  size_t     route_len;
  double     stop_lat;
  double     stop_lon;
} DeviceConfig;

static void *run_device(void *arg) {
    DeviceConfig *cfg = (DeviceConfig *)arg;

    int sock;
    struct sockaddr_in6 server;
    unsigned char buffer[BUFFER_SIZE];

    sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "[device %d] socket() failed\n", cfg->device_id);
        return NULL;
    }

    server.sin6_port   = htons(SERVER_PORT);
    server.sin6_family = AF_INET6;
    inet_pton(AF_INET6, SERVER_IP, &server.sin6_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "[device %d] connection failed\n", cfg->device_id);
        close(sock);
        return NULL;
    }
    printf("[device %d] connected\n", cfg->device_id);

    /* --- IMEI handshake --- */
    int imei_len = strlen(cfg->imei);
    unsigned char imei_packet[32];
    imei_packet[0] = 0;
    imei_packet[1] = imei_len;
    memcpy(imei_packet + 2, cfg->imei, imei_len);
    send_all(sock, imei_packet, imei_len + 2);

    read_exact(sock, buffer, 1);
    if (buffer[0] != 1) {
        printf("[device %d] IMEI rejected\n", cfg->device_id);
        close(sock);
        return NULL;
    }
    printf("[device %d] IMEI accepted\n", cfg->device_id);

    /* --- First AVL packet --- */
    unsigned char avl_packet[AVL_PACKET_SIZE] = {
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
      0x05,
      0x00,
      0x00,
      0x00,
      0x00,
      0x00,
      0x00,
      0x01,
      0x00,0x00,0x00,0x00
    };

    send_all(sock, avl_packet, sizeof(avl_packet));
    printf("[device %d] socket fd = %d\n", cfg->device_id, sock);

    int n = read_exact(sock, buffer, 4);
    printf("[device %d] ACK: %02X %02X %02X %02X\n",
           cfg->device_id,
           buffer[0], buffer[1], buffer[2], buffer[3]);
    if (n == 4)
        printf("[device %d] server acknowledged\n", cfg->device_id);
    else
        printf("[device %d] server did NOT acknowledge\n", cfg->device_id);

    simulate_movement(cfg->device_id, &sock, avl_packet, buffer, cfg->route, cfg->route_len);
    simulate_stop(cfg->device_id, &sock, avl_packet, buffer, cfg->stop_lat, cfg->stop_lon);

    sleep(2);
    close(sock);
    printf("[device %d] done\n", cfg->device_id);
    return NULL;
}


int main(){

  DeviceConfig devices[] = {
    { "123456789012345", 9, route1, sizeof(route1)/sizeof(route1[0]), -8.11160, -79.02240 },
    { "243210987654321", 1, route2, sizeof(route2)/sizeof(route2[0]), -8.13000, -79.02800 },
  };
    
  int n_devices = sizeof(devices) / sizeof(devices[0]);
  pthread_t threads[n_devices];
  
  for (int i = 0; i < n_devices; i++) {
    if (pthread_create(&threads[i], NULL, run_device, &devices[i]) != 0) {
        fprintf(stderr, "Failed to create thread for device %d\n", i + 1);
    }
  }

  for (int i = 0; i < n_devices; i++) {
      pthread_join(threads[i], NULL);
  }


  return 0;
}
