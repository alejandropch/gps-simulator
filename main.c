#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "utils/utils.h"

#define SERVER_IP "::1"
#define SERVER_PORT 5027

int main(){

  int sock;
  struct sockaddr_in6 server;
  unsigned char buffer[BUFFER_SIZE];
  sock = socket(AF_INET6, SOCK_STREAM, 0);
  server.sin6_port = htons(SERVER_PORT);
  server.sin6_family = AF_INET6;
  inet_pton(AF_INET6, SERVER_IP, &server.sin6_addr);

  if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
    perror("connection failed\n");
    return 1;
  }
  printf("connected\n");
  char imei[] = "123456789012345";
  int imei_len = strlen(imei);

  unsigned char imei_packet[32];
  imei_packet[0] = 0;
  imei_packet[1] = imei_len;
  memcpy(imei_packet + 2, imei, imei_len);
  send(sock, imei_packet, imei_len + 2, 0);
  recv(sock, buffer, 1, 0);
  if(buffer[0] != 1){
    printf("IMEI rejected\n");
    return 1;
  }
  printf("IMEI accepted\n");

  unsigned char avl_packet[AVL_PACKET_SIZE] = {

      0x00,0x00,0x00,0x00,   // preamble
      0x00,0x00,0x00,0x21,   // data length = 30
      0x08,                  // codec
      0x01,                  // records

      // timestamp - 8 bytes - index [10, 17]
      0x00,0x00,0x01,0x94,0x4A,0xC0,0x00,0x00,

      0x01,                  // priority

      // longitude - 4 bytes - index [19, 22]
      0xF8,0xA4,0xA1,0xB0,

      // latitude - 4 bytes - index [23, 26]
      0xFF,0x72,0x8B,0x50,

      0x00,0xB4,             // altitude - index [27, 28]
      0x01,0x2C,             // angle - index [29, 30]
      0x05,                  // satellites - index [31]
      0x00,0x00,             // speed - index [32 - 33]

      0x00,                  // event id
      0x00,                  // total IO

      0x00,                  // N1 - index [36 -
      0x00,                  // N2
      0x00,                  // N4
      0x00,                  // N8   39]

      0x01,                  // NOD2 [40]

      0x00,0x00,0x00,0x00    // CRC placeholder - 4 bytes - index [41 - 44]
  };
  send(sock, avl_packet, sizeof(avl_packet), 0);
  printf("AVL packet sent\n");
  int n = recv(sock, buffer, 4, 0);
  printf("ACK: %02X %02X %02X %02X\n",
    buffer[0],buffer[1],buffer[2],buffer[3]
  );
  if (n == 4) {
    printf("server acknowledged\n");
  } else{
    printf("server did NOT acknowledged\n");
  } 

  simulate_movement(&sock, avl_packet, buffer);
  simulate_stop(&sock, avl_packet, buffer);

  sleep(2);



  close(sock);
  return 0;
}
