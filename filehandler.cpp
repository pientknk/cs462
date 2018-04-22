#ifndef FILEHANDLER
#define FILEHANDLER

#include "filehandler.h"

//return a string of all characters in the designated file
char* ParseFile(string fileName, long fileSize) {
	FILE *filePtr;
	filePtr = fopen(fileName.c_str(), "rb");
	if (filePtr == NULL) {
		cerr << "ERROR opening file: " << fileName << endl;
		return NULL;
	}
	else {
		char *buffer = new char[fileSize];
		if (buffer == NULL) {
			cerr << "Memory Error" << endl;
			return NULL;
		}
		else {
			size_t result = fread(buffer, 1, fileSize, filePtr);
			if (result != fileSize) {
				cerr << "Reading File Error" << endl;
				return NULL;
			}
			else {
				return buffer;
			}
		}
	}

	fclose(filePtr);
}

long GetFileSize(string fileName) {
	ifstream myfile(fileName, ifstream::binary);
	if (myfile.good() && myfile.is_open()) {
		myfile.seekg(0, myfile.end);
		return myfile.tellg();
	}
	else {
		cout << "ERROR: Could not open file" << endl;
		return -1;
	}
}

#endif