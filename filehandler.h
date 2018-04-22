#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

char* ParseFile(string fileName, long fileSize);

long GetFileSize(string fileName);

#endif