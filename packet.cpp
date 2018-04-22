#ifndef PACKET
#define PACKET

#include "timer.cpp"

typedef struct Packet {
	int SOHIndex = 0;
	int seqNumIndex = 1;
	int SeqNum;
	int STXIndex = 2;
	const char* data;
	int PayloadIndex = 3;
	suseconds_t startTimeUSecs;
} Packet;

#endif
