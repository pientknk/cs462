#ifndef USERINPUT_H
#define USERINPUT_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "constants.h"

using namespace std;

enum Protocol { SW, GBN, SR }; //Stop and Wait, Go-Back-N, Selective Repeat
const string Protocols[] = { "0", "1", "2" };

enum TOInterval { US, PC }; //User Specified, Ping Calculated
const string TOIntervals[] = { "0", "1" };

enum SitError { USP, RG, NO,  }; // None, Randomly Generated, User Specified
const string SitErrors[] = { "0", "1", "2" };

enum ErrorControl { PL, PD, AL, ML }; //Packet Loss, Packet Damage, Ack Lost, Multiple
const string ErrorControls[] = { "0", "1", "2", "3" };

string GetPresets();

string GetFileName();

int GetPacketSize();

int GetSequenceNumberRange(int maxPackets);

int GetProtocol();

int GetTimeOutIntervalMethod();

int GetTimeOutFromUser();

int GetSlidingWindowSize(int maxPackets);

int GetSituationalErrorType();

int GetErrorControlType();

int GetNumberOfPacketsToDropOrDamage(int maxNumberOfPackets, bool isDropping);

vector<int> GetAllPacketsToDropOrDamage(int numPacketsToDrop, bool isDropping);

int GetNumberOfAcksToLose(int maxNumberOfPackets);

vector<int> GetAllAcksToLose(int numAcksLost);

void GetMultipleErrorsFromUser(int maxNumberOfPackets, vector<int>& droppedPackets, vector<int>& damagedPackets, vector<int>& acksLost);

#endif
