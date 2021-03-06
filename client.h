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
#include <algorithm>
#include <deque>

#include "timer.cpp"
#include "filehandler.h"
#include "crc16Checksum.h"
#include "constants.h"
#include "userinput.h"


using namespace std;

string executeCommand(string cmd);

int callServer(string host, int portNum);

int writePacket(vector<int> &acksToLose, vector<int> &packetsToDamage, vector<int> &packetsToDrop, deque<unsigned char*> *windowContents, unsigned char* packetPointer);

void client(int portNum, int packetSize, int seqNumberRange, string fileName, int protocol, int slidingWindowSize, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout);

void clientStopAndWait(int portNum, int packetSize, int seqNumRange, string fileName, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout);

void clientGBN(int portNum, int packetSize, int seqNumberRange, string fileName, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout, int slidingWindowSize);

#endif
