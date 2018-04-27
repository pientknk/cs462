#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <limits.h>

const char NUL = 0x00; //null ascii value
const char SOH = 0x01; //Start of Header
const char STX = 0x02; //Start of Text
const char ETX = 0x03; //End of Text
const char EOP = 0x7E; //End of Packet
const char DLE = 0x10; //Data Link Escape
const char ACK = 0x06; //ACK Ascii character, also (0000 0110)
const char USCR = 0x5F; //Underscore Character
const int MAX_PACKET_SIZE = 60000; //payload size
const int PACKET_FRAME_SIZE = 6; //the amount of bytes needed to frame the packet
const int MAX_SEQ_NUM_RANGE = 254;
const int MAX_TIMEOUT = 5000000; // the max amount of microseconds for a timeout
const int MIN_TIMEOUT = 1; // the min time for a packet timeout in microseconds
const int DEFAULT_PACKET_SIZE = 512;
const int DEFAULT_SEQ_NUM_RANGE = 32;
const int DEFAULT_TIMEOUT = 2000000;
const int MAX_TRANSMISSION_ATTEMPTS = 5;

#endif