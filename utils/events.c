#include "utils.h"
#include <bits/types/struct_iovec.h>

void simulate_movement(int *sock, unsigned char avl_packet[AVL_PACKET_SIZE], unsigned char buffer[BUFFER_SIZE]){

  printf("\n SIMULATING MOVEMENT \n");

  int n = 0;
  double route[][2] = {
    {-8.10980, -79.02420},
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
    {-8.11060, -79.02100}
  };
  for(size_t i = 0; i < (sizeof(route)/sizeof(route[0])); i++){
    printf("sending lat=%.3f lon=%.3f\n", route[i][0], route[i][1]);
    long long ts = current_time_ms();
    int lat = route[i][0] * 10000000;
    int lon = route[i][1] * 10000000;

    write_int64(&avl_packet[10], ts);
    write_int32(&avl_packet[19], lon);
    write_int32(&avl_packet[23], lat);


    double dlat = (route[i][0] - route[i > 0 ? i-1 : 0][0]) * 111000.0;
    double dlon = (route[i][1] - route[i > 0 ? i-1 : 0][1]) * 111000.0 * cos(route[i][0] * M_PI / 180.0);
    double dist_m = sqrt(dlat*dlat + dlon*dlon);
    uint16_t speed_kmh = (uint16_t)((dist_m / 3.0) * 3.6); // 3s interval

    avl_packet[32] = (speed_kmh >> 8) & 0xFF;
    avl_packet[33] = speed_kmh & 0xFF;


    uint32_t crc =  crc16_teltonika(&avl_packet[8], 30);
    write_crc(&avl_packet[41], crc);
    send(*sock, avl_packet, AVL_PACKET_SIZE, 0);

    n = recv(*sock, buffer, 4, 0);

    if (n == 4) {
      printf("Server acknowledged: %d\n",
        (buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|buffer[3]);
    } else 
      printf("server did NOT acknowledged\n");

    sleep(3);
  }
}


void simulate_stop(int *sock, unsigned char avl_packet[AVL_PACKET_SIZE], unsigned char buffer[BUFFER_SIZE]){
  printf("\n SIMULATING GEOFENCE STOP \n");

  int n = 0;

  // these coordinates are inside my geofence
  double stop_lat = -8.11160;
  double stop_lon = -79.02240;

  int slat = stop_lat * 10000000;
  int slon = stop_lon * 10000000;

  // ssetting speed to 0
  avl_packet[32] = 0x00;
  avl_packet[33] = 0x00;

  write_int32(&avl_packet[19], slon);
  write_int32(&avl_packet[23], slat);

  // 3 packets: entry, stopped, stopped 11 min later
  for (int i = 0; i < 3; i++) {
      long long ts = current_time_ms();
      write_int64(&avl_packet[10], ts);

      uint32_t crc = crc16_teltonika(&avl_packet[8], 30);
      write_crc(&avl_packet[41], crc);

      send(*sock, avl_packet, AVL_PACKET_SIZE, 0);
      n = recv(*sock, buffer, 4, 0);

      if (n == 4)
        printf("server acknowledged\n");
      else 
        printf("server did NOT acknowledged\n");

      printf("Stop packet %d sent (t+%lld ms), ACK: %d\n", i,
          ts,
          (buffer[0]<<24)|(buffer[1]<<16)|(buffer[2]<<8)|buffer[3]);

      sleep(2); // small real delay between sends
  }

  printf("\n RESETTING - sending movement packet\n");
  avl_packet[32] = 0x00;
  avl_packet[33] = 0xA; // speed = 10 kmh

  long long ts = current_time_ms();
  write_int64(&avl_packet[10], ts);
  write_int32(&avl_packet[19], slon);
  write_int32(&avl_packet[23], slat);
  uint32_t crc = crc16_teltonika(&avl_packet[8], 30);
  write_crc(&avl_packet[41], crc);
  send(*sock, avl_packet, AVL_PACKET_SIZE, 0);
  n = recv(*sock, buffer, 4, 0);
  if (n == 4)
    printf("server acknowledged\n");
  else 
    printf("server did NOT acknowledged\n");
  printf("Movement reset packet sent\n");
}
