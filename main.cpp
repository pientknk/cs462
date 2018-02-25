#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "process.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits.h>
#include <algorithm>
#include <queue>
#include <math.h>
#include <deque>
#include <map>

using namespace std;

//do we want this to be a class?
typedef struct Packet {
	uint8_t sof;
	uint32_t src;
	uint32_t dst;
	uint8_t seqn;
	byte[] payload;
	uint32_t cksum;
	uint8_t eof;
};

int main() {
	// GET Packet size from user
	bool validPacketSize = false;
	string packetSize;
	while (!validPacketSize) {
		cout << "Please specify a packet size in bytes: " << endl;
		cin >> packetSize;
		if (packetSize >= 100 || packetSize <= 300) {
			validPacketSize = true;
		}
		else {
			cout << "\nERROR: Packet size must be between 100 and 300 bytes, but you said: '" << packetSize << "'" << endl;
		}
	}

	// GET range of sequence numbers from user
	bool choseSeqNumRange = false;
	string seqNumRange;
	while (!choseSeqNumRange) {
		cout << "Please specify the range of sequence numbers: " << endl;
		cin >> seqNumRange;
		if (seqNumRange >= 1 || seqNumRange <= INTMAX_C) {
			choseSeqNumRange = true;
		}
		else {
			cout << "\nERROR: Sequence number range must be within 1-" << INTMAX_C << ", but you said: '" << seqNumRange << "'" << endl;
		}
	}

	exit(0);
}

void printStats(long packetSize, int numPacketsSent, int numPacketsReceived, double totalTime, double throughput, string md5Sum) {
	cout << endl << "Packet Size: " << packetSize << "bytes" << endl;
	if (numPacketsSent != 0) {
		cout << << "Number of packets sent: " << numPacketsSent << endl;
	}
	else {
		cout << << "Number of packets received: " << numPacketsReceived << endl;
	}

	cout << endl << "Total elapsed time: " << totalTime << endl;
	cout << "Throughput (Mbps): " << throughput << endl;
	cout << "md5sum: " << md5Sum << endl;boosrt 
}

void createPacket() {
	//Start of frame (SOF) (8 bits)
	//SRC IPv4 (32 bits)
	//DST IPV4 (32 bits)
	//SEQ# (8 bits)
	//Payload (100 bytes default)
	//Checksum (32 bits cuz it takes 17?)
	//End of Frame (EOF) (8 bits)
}

//CRC16 checksum (it's actually 17 bits?) from https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum

#define CRC16 0x8005

uint16_t gen_crc16(const uint8_t *data, uint16_t size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if (data == NULL)
		return 0;

	while (size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

										 /* Increment bit counter: */
		bits_read++;
		if (bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		/* Cycle check: */
		if (bit_flag)
			out ^= CRC16;

	}

	// item b) "push out" the last 16 bits
	int i;
	for (i = 0; i < 16; ++i) {
		bit_flag = out >> 15;
		out <<= 1;
		if (bit_flag)
			out ^= CRC16;
	}

	// item c) reverse the bits
	uint16_t crc = 0;
	i = 0x8000;
	int j = 0x0001;
	for (; i != 0; i >>= 1, j <<= 1) {
		if (i & out) crc |= j;
	}

	return crc;
}
