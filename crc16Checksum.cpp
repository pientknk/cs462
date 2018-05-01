//Taken from https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum
#ifndef CHECKSUM
#define CHECKSUM

#define CRC16 0x8005

#include "crc16Checksum.h"

uint16_t gen_crc16(const unsigned char* data_p, uint16_t length) {
	unsigned char x;
	unsigned short crc = 0xFFFF;

	while (length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
	}
	return crc;
}

#endif