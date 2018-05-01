#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <cstdint>
#include <cstddef>

uint16_t gen_crc16(const unsigned char* data_p, uint16_t length);

#endif