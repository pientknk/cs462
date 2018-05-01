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

#include "timer.cpp"
#include "filehandler.h"
#include "crc16Checksum.h"
#include "constants.h"

using namespace std;

void readPacket();

void generatePacket();

int serverSocketSetup(int portNum);

int serverSocketAccept(int serverSocket);

void server(int portNum);

void serverStopAndWait(int portNum, int ss, int seqNumRange);

#endif