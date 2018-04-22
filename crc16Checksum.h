#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <cstdint>
#include <cstddef>

uint16_t gen_crc16(const char *data, uint16_t size);

#endif