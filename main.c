#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "::1"
#define SERVER_PORT 5027

int main(){

  int sock;
  struct sockaddr_in6 server;
  unsigned char buffer[1024];
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

      // --- SIMPLE AVL PACKET (Codec 8) ---
    unsigned char avl_packet[] = {

        0x00,0x00,0x00,0x00,   // preamble
        0x00,0x00,0x00,0x2D,   // data length

        0x08,                  // codec
        0x01,                  // records

        // timestamp
        0x00,0x00,0x01,0x94,0x4A,0xC0,0x00,0x00,

        0x01,                  // priority

        // longitude
        0xF8,0xA4,0xA1,0xB0,

        // latitude
        0xFF,0x72,0x8B,0x50,

        0x00,0xB4,             // altitude
        0x01,0x2C,             // angle
        0x05,                  // satellites
        0x00,0x00,             // speed

        0x00,                  // event id
        0x00,                  // total IO

        0x00,0x00,0x00,0x00    // CRC placeholder
    };

    send(sock, avl_packet, sizeof(avl_packet), 0);
    printf("AVL packet sent\n");
    recv(sock, buffer, 4, 0);
    printf("server acknowledged\n");
    close(sock);
    return 0;


}
