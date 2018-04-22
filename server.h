#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

#include "packet.cpp"
#include "timer.cpp"
#include "files.h"
#include "crc16Checksum.h"

#ifndef CONSTANTS
#define CONSTANTS
const char NUL = 0x00; //null ascii value
const char SOH = 0x01; //Start of Header
const char STX = 0x02; //Start of Text
const char ETX = 0x03; //End of Text
const char EOP = 0x7E; //End of Packet
const char DLE = 0x10; //Data Link Escape
const char ACK = 0x06; //ACK Ascii character, also (0000 0110)
const int MAX_PACKET_SIZE = 2048; //payload size
const int PACKET_FRAME_SIZE = 6; //the amount of bytes needed to frame the packet
const int MAX_SEQ_NUM_RANGE = CHAR_MAX;
#endif

using namespace std;

void readPacket();

void generatePacket();

int serverSocketSetup(int portNum);

int serverSocketAccept(int serverSocket);

void server(int portNum);

#endif