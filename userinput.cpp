string GetFileName() {
	bool validFile = false;
	string fileName;
	while (!validFile) {
		cout << "Please give the name of the file to send: " << endl;
		cin >> fileName;
		ifstream myfile(fileName);
		if (myfile.good() && myfile.is_open()) {
			validFile = true;
		}
		else {
			cout << "\nERROR: unable to open file: " << fileName << endl;
		}
	}

	return fileName;
}

int GetPacketSize() {
	size_t* st;
	int packetSizeInBytes = 0;
	// GET Packet size from user
	bool validPacketSize = false;
	string tempPacketSize;
	while (!validPacketSize) {
		cout << "Please specify a packet size in bytes: " << endl;
		cin >> tempPacketSize;
		packetSizeInBytes = stoi(tempPacketSize, st, 10);
		if (packetSizeInBytes >= 1 && packetSizeInBytes <= MAX_PACKET_SIZE - PACKET_FRAME_SIZE) {
			validPacketSize = true;
		}
		else {
			cout << "\nERROR: Packet size must be between 1 and " << MAX_PACKET_SIZE - PACKET_FRAME_SIZE << " bytes" << endl;
		}
	}

	return packetSizeInBytes;
}

int GetSequenceNumberRange() {
	bool choseSeqNumRange = false;
	int seqNumberRange = 0;
	string seqNumRange;
	size_t* st;
	while (!choseSeqNumRange) {
		cout << "Please specify the range of sequence numbers: " << endl;
		cin >> seqNumRange;
		seqNumberRange = stoi(seqNumRange, st, 10);
		if (seqNumberRange >= 1 && seqNumberRange <= MAX_SEQ_NUM_RANGE) {
			choseSeqNumRange = true;
		}
		else {
			cout << "\nERROR: Sequence number range must be within 1-" << MAX_SEQ_NUM_RANGE << ", but you said: '" << seqNumberRange << "'" << endl;
		}
	}

	return seqNumberRange;
}

int GetProtocol() {
	bool choseProtocol = false;
	string prot;
	int protocol = Protocol::SW;
	size_t* st;
	while (!choseProtocol) {
		cout << "Please select a protocol. 0 for SW, 1 for GBN, or 2 for SR" << endl;
		cin >> prot;
		if (prot == Protocols[Protocol::SW] || prot == Protocols[Protocol::GBN] || prot == Protocols[Protocol::SR]) {
			protocol = stoi(prot, st, 10);
			choseProtocol = true;
		}
		else {
			cout << "\nERROR: You must select protocol 0, 1, or 2" << endl;
		}
	}

	return protocol;
}

int GetTimeOutIntervalMethod() {
	bool choseInterval = false;
	string interval;
	int theInterval = TOInterval::PC;
	size_t* st;
	while (!choseInterval) {
		cout << "Please select a Time Out Interval Method. 0 for User-Specified, or 1 for Ping-Calculated" << endl;
		cin >> interval;
		if (interval == TOIntervals[TOInterval::PC] || interval == TOIntervals[TOInterval::US]) {
			theInterval = stoi(interval, st, 10);
			choseInterval = true;
		}
		else {
			cout << "\nERROR: You must select Time Out Interval Method 0, or 1" << endl;
		}
	}

	return theInterval;
}

int GetTimeOutFromUser() {
	bool choseInterval = false;
	string interval;
	int theInterval = 1;
	size_t* st;
	while (!choseInterval) {
		cout << "How long should the timeout be in microseconds?" << endl;
		cin >> interval;
		if (isDigit(interval)) {
			theInterval = stoi(interval, st, 10);
			if (theInterval > 0 && theInterval <= 5000000) {
				choseInterval = true;
			}
			else {
				cout << "\nERROR: You must enter a number that is between 1 and 5,000,000 (5 sec) microseconds" << endl;
			}
		}
		else {
			cout << "\nERROR: You must enter a number" << endl;
		}
	}

	return theInterval;
}

int GetSlidingWindowSize() {
	bool choseSlidingWindowSize = false;
	string inputSize;
	int size = 1;
	size_t* st;
	while (!choseSlidingWindowSize) {
		cout << "What should the Sliding Window Size be?" << endl;
		cin >> interval;
		if (isDigit(interval)) {
			size = stoi(interval, st, 10);
			if (size > 0 && size <= 16) {
				if (size != 1 && size % 2 != 0) {
					cout << "\nERROR: You must enter a number that is between 1 and 16 and a multiple of 2" << endl;
				}
				else {
					choseInterval = true;
				}
			}
			else {
				cout << "\nERROR: You must enter a number that is between 1 and 16 and a multiple of 2" << endl;
			}
		}
		else {
			cout << "\nERROR: You must enter a number" << endl;
		}
	}

	return size;
}

int GetSituationalErrorType() {
	bool choseSitErrorType = false;
	string sitErrorType;
	int errorType = SitError::NO;
	size_t* st;
	while (!choseSitErrorType) {
		cout << "Please select a Situational Error Type. 0 for None, 1 for Randomly Generated, or 2 for User-Specified" << endl;
		cin >> sitErrorType;
		if (sitErrorType == SitErrors[SitError::NO] || sitErrorType == SitErrors[SitError::RG] || sitErrorType == SitErrors[SitError::US]) {
			errorType = stoi(sitErrorType, st, 10);
			choseSitErrorType = true;
		}
		else {
			cout << "\nERROR: You must select a Situational Error Type 0, 1, or 2" << endl;
		}
	}

	return errorType;
}

int GetErrorControlType() {
	bool choseErrorControlType = false;
	string errorControlType;
	int errorType = ErrorControl::AL;
	size_t* st;
	while (!choseErrorControlType) {
		cout << "Please select an Error Control Type. 0 for Packet Loss, 1 for Packet Damage, or 2 for Ack Loss or 3 for multiple" << endl;
		cin >> errorControlType;
		if (errorControlType == ErrorControls[ErrorControl::AL] || errorControlType == ErrorControls[ErrorControl::ML] || errorControlType == ErrorControls[ErrorControl::PD] || errorControlType == ErrorControls[ErrorControl::PL]) {
			errorType = stoi(errorControlType, st, 10);
			choseErrorControlType = true;
		}
		else {
			cout << "\nERROR: You must select an Error Control Type 0, 1, 2, or 3" << endl;
		}
	}

	return errorType;
}



//TO RUN SERVER: ./packet 9036 -s
//TO RUN CLIENT: ./packet 9036 -c
//int main(int argc, char **argv) {
//	if (argc == 3) {
//		int packetSizeInBytes = MAX_PACKET_SIZE;
//		int seqNumberRange = MAX_SEQ_NUM_RANGE;
//		char* ptr;
//		unsigned long portNum = strtol(argv[1], &ptr, 10);
//		if (portNum <= 9000 || portNum > 10000) {
//			cerr << "ERROR with second parameter: " << argv[1] << ", it should be a valid port number between 9,000 and 10,000" << endl;
//		}
//		else {
//			//if this process should be the server
//			if (!strcmp(argv[2], "-s")) {
//				server(portNum);
//			}
//			else if (!strcmp(argv[2], "-c")) {
//				string fileName = GetFileName();
//
//				int protocol = GetProtocol();
//				int slidingWindowSize = 0;
//				if (protocol == Protocol::GBN || protocol == Protocol::SR) {
//					slidingWindowSize = GetSlidingWindowSize()
//				}
//
//				packetSizeInBytes = GetPacketSize();
//
//				int intervalMethod = GetTimeOutIntervalMethod();
//				int intervalTimeInMicroseconds = -1;
//				if (intervalMethod == TOInterval::US) {
//					intervalTimeInMicroseconds = GetTimeOutFromUser();
//				}
//				else {
//					//use ping to pick a good timeout? or pass in -1 so that the client know that it needs to figure it out
//				}
//
//				seqNumberRange = GetSequenceNumberRange();
//
//				int situationalErrorType = GetSituationalErrorType();
//				if (situationalErrorType == SitErrors[SitError::US]) {
//					int errorControlType = GetErrorControlType();
//					//ack lost
//					if (errorControlType == ErrorControls[ErrorControl::AL]) {
//
//					}
//					//packet damage
//					else if (errorControlType == ErrorControls[ErrorControl::PD]) {
//
//					}
//					//packet loss
//					else if (errorControlType == ErrorControls[ErrorControl::PL]) {
//
//					}
//					//multiple
//					else {
//
//					}
//				}
//
//				client(portNum, packetSizeInBytes, seqNumberRange, fileName);
//			}
//			else {
//				cerr << "ERROR with third parameter: " << argv[2] << ", the proper flags are -c or -s" << endl;
//			}
//		}
//	}
//	else {
//		cerr << "ERROR: Incorrect number of arguments. Command should be something like ./packet portNum -flag (s or c)" << endl;
//	}
//
//	exit(0);
//}