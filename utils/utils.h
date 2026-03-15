#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>
void write_int64(unsigned char *buf, int64_t value);
void write_int32(unsigned char *buf, int32_t value);
void write_crc(unsigned char *buf, int16_t value);
uint16_t crc16_teltonika(const uint8_t *data, int len);
long long current_time_ms();
#endif
