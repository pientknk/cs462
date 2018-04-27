#ifndef PRINTING_H
#define PRINTING_H

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "constants.h"

using namespace std;

void printClientStats(int bytesWritten, int bytesResent, int numPacketsSent, double throughput, double UDuration, const char* MD5Command);

void printServerStats(int lastPacketNum, int bytesReceived, int bytesDuplicate, int numPacketsReceived, double UDuration);

void printCheckSumIndividual(uint16_t value);

void printCheckSum(uint16_t value);

void printPacket(int packetBytes, unsigned char* packetbBuffer);

void printPacketReplace(int packetBytes, unsigned char* packetBuffer);

void printPayload(int packetSize, unsigned char* payload);

void printPayloadReplace(int packetSize, unsigned char* payload);

#endif
