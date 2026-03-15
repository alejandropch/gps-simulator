#include "utils.h"
#include <bits/time.h>
#include <stdint.h>
#include <time.h>

void write_int64(unsigned char *buf, int64_t value){
  buf[0] = (value >> 56) & 0XFF;
  buf[1] = (value >> 48) & 0XFF;
  buf[2] = (value >> 40) & 0XFF;
  buf[3] = (value >> 32) & 0XFF;
  buf[4] = (value >> 24) & 0XFF;
  buf[5] = (value >> 16) & 0XFF;
  buf[6] = (value >> 8) & 0XFF;
  buf[7] = value & 0XFF;
};
void write_int32(unsigned char *buf, int32_t value){
    buf[0] = (value >> 24) & 0xFF;
    buf[1] = (value >> 16) & 0xFF;
    buf[2] = (value >> 8) & 0xFF;
    buf[3] = value & 0xFF;
};

void write_crc(unsigned char *buf, int16_t value){
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = (value >> 8) & 0xFF;
    buf[3] = value & 0xFF;
};

long long current_time_ms(){
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

uint16_t crc16_teltonika(const uint8_t *data, int len) {
    uint16_t crc = 0x0000;

    for (int i = 0; i < len; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}

