#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define AVL_PACKET_SIZE 45
#define BUFFER_SIZE 1024
void write_int64(unsigned char *buf, int64_t value);
void write_int32(unsigned char *buf, int32_t value);
void write_crc(unsigned char *buf, int16_t value);
uint16_t crc16_teltonika(const uint8_t *data, int len);
long long current_time_ms();
void simulate_movement(int *sock, unsigned char avl_packet[AVL_PACKET_SIZE], unsigned char buffer[BUFFER_SIZE]);
#endif
