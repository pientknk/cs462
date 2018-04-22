#ifndef USERINPUT_H
#define USERINPUT_H

#include <string>
#include <iostream>
#include <fstream>

#include "constants.h"

using namespace std;

enum Protocol { SW, GBN, SR }; //Stop and Wait, Go-Back-N, Selective Repeat
const string Protocols[] = { "0", "1", "2" };

enum TOInterval { US, PC }; //User Specified, Ping Calculated
const string TOIntervals[] = { "0", "1" };

enum SitError { NO, RG, USP }; // None, Randomly Generated, User Specified
const string SitErrors[] = { "0", "1", "2" };

enum ErrorControl { PL, PD, AL, ML }; //Packet Loss, Packet Damage, Ack Lost, Multiple
const string ErrorControls[] = { "0", "1", "2" };

string GetFileName();

int GetPacketSize();

int GetSequenceNumberRange();

int GetProtocol();

int GetTimeOutIntervalMethod();

int GetTimeOutFromUser();

int GetSlidingWindowSize();

int GetSituationalErrorType();

int GetErrorControlType();

#endif
