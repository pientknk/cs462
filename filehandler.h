#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

char* parseFile(string fileName, long fileSize);

long getFileSize(string fileName);

#endif