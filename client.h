#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "packet.cpp"
#include "timer.cpp"
#include "filehandler.h"
#include "crc16Checksum.h"
#include "constants.h"
#include "userinput.h"


using namespace std;

int callServer(string host, int portNum);

int writePacket();

void client(int portNum, int packetSize, int seqNumberRange, string fileName, int protocol, int slidingWindowSize, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop);

#endif
