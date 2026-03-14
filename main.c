#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5027

int main(){

  int sock;
  struct sockaddr_in server;
  unsigned char buffer[1024];
  sock = socket(AF_INET, SOCK_STREAM, 0);
  server.sin_addr.s_addr = inet_addr(SERVER_IP);
  server.sin_family = htons(SERVER_PORT);

  if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
    printf("connection failed\n");
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
}
