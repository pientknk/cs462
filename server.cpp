#include "server.h"

int sockfd;
string packet_s;
char* packet2;
char packetNum;
int packetBytes;
char* packetBuffer;
int packetBufferIndex;
int payloadSize;
int sequenceNumber_s;
int bytes_s;
int totalBytes_s;
bool foundEndOfPacket;
bool isDone;

//initialize socket for the server
int serverSocketSetup(int portNum) {
	//get the right socket file descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("ERROR opening this socket\n");
		exit(1);
	}

	// server address structure
	struct sockaddr_in serv_addr;

	//set all the valies in the server address to 0
	memset(&serv_addr, '0', sizeof(serv_addr));

	//setup the type of socket to be internet
	serv_addr.sin_family = AF_INET;

	//address of the machine we are on
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//setup the port number
	serv_addr.sin_port = htons(portNum);

	//bind the socket to the given port
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("ERROR with binding\n");
		exit(1);
	}

	return sockfd;
}

int serverSocketAccept(int serverSocket)
{
	int sockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);

	cout << "Waiting for a call..." << endl;
	sockfd = accept(serverSocket, (struct sockaddr *) &cli_addr, &clilen);
	cout << "Connected" << endl;
	if (sockfd < 0) {
		cerr << "ERROR accepting connection" << endl;
		exit(1);
	}

	return sockfd;
}

void readPacket() {
	while (!foundEndOfPacket) {
		bytes_s = read(sockfd, packetBuffer + packetBufferIndex, 1); //if slow, read in as much as we can and loop through the buffer to see if there is an ETX
		if (bytes_s <= 0) {
			cout << "1. ERROR reading from socket: " << sockfd << endl;
			isDone = true;
			break;
		}
		else {
			packetBytes += bytes_s;
			totalBytes_s += bytes_s;


			//get current char to check if this is the end of the packet
			char currentChar = *(packetBuffer + packetBufferIndex);
			if (currentChar == ETX && *(packetBuffer + packetBufferIndex - 1) != DLE) {
				foundEndOfPacket = true;

				//read in the checksum
				bytes_s = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
				packetBytes += bytes_s;
				totalBytes_s += bytes_s;
				packetBufferIndex += bytes_s;
			}

			packetBufferIndex++;
			if (packetBytes == 2) {
				packetNum = currentChar;
			}
		}
	}
}

void generatePacket() {
	//get accurate payload size after checking for DLE's
	payloadSize = packetBytes - PACKET_FRAME_SIZE;
	//cout << "Payload size: " << payloadSize << endl;
	int adjustedPayloadSize = 0;
	for (int i = 3; i < payloadSize + 3; i++) {
		char packetChar = *(packetBuffer + i);
		if (packetChar == DLE) {
			packetChar = *(packetBuffer + i + 1);
			if (packetChar == DLE) {
				adjustedPayloadSize++;
			}
		}
		else {
			adjustedPayloadSize++;
		}
	}

	//create string from buffer somehow
	packet2 = new char[adjustedPayloadSize];
	int packet2Index = 0;
	for (int i = 3; i < payloadSize + 3; i++) {
		char packetChar = *(packetBuffer + i);
		if (packetChar == DLE) {
			i++;
			packetChar = *(packetBuffer + i);
			packet_s += packetChar;
			packet2[packet2Index] = packetChar;
			packet2Index++;
		}
		else if (!(packetChar == SOH || packetChar == STX || packetChar == ETX)) {
			packet_s += packetChar;
			packet2[packet2Index] = packetChar;
			packet2Index++;
		}
	}
}

void sendAck() {
	
}

void server(int portNum)
{
	sequenceNumber_s = 0;
	int numPacketsReceived = 0;
	totalBytes_s = 0;
	bytes_s = 0;
	packetBytes = 0;

	int ss = serverSocketSetup(portNum);
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);

	//Time variables
	Timer timer;
	time_t startTime = timer.GetTimeInSeconds();
	suseconds_t startTimeUSecs = timer.GetTimeInMicroSeconds();
	double duration = 0.0;

	//Buffer
	packetBuffer = new char[int(MAX_PACKET_SIZE * 1.5) + PACKET_FRAME_SIZE];

	//read in the sequence number range
	int seqNumRange = 0;
	int* seqNumRangePointer = &seqNumRange;
	if (!read(sockfd, seqNumRangePointer, sizeof(int))) {
		//cout << "1. ERROR reading from socket: " << sockfd << endl;
	}

	ofstream myfile;
	myfile.open("Output.txt");

	isDone = false;
	while (!isDone) {
		/**** READING ****/
		cout << "Expected seq#: " << sequenceNumber_s << endl;
		foundEndOfPacket = false;
		packetBufferIndex = 0;
		packetBytes = 0;
		packetNum = 'Z';
	
		//readPacket();
		while (!foundEndOfPacket) {
		bytes_s = read(sockfd, packetBuffer + packetBufferIndex, 1); //if slow, read in as much as we can and loop through the buffer to see if there is an ETX
		if (bytes_s <= 0) {
			cout << "1. ERROR reading from socket: " << sockfd << endl;
			isDone = true;
			break;
		}
		else {
			packetBytes += bytes_s;
			totalBytes_s += bytes_s;


			//get current char to check if this is the end of the packet
			char currentChar = *(packetBuffer + packetBufferIndex);
			if (currentChar == ETX && *(packetBuffer + packetBufferIndex - 1) != DLE) {
				foundEndOfPacket = true;

				//read in the checksum
				bytes_s = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
				packetBytes += bytes_s;
				totalBytes_s += bytes_s;
				packetBufferIndex += bytes_s;
			}

			packetBufferIndex++;
			if (packetBytes == 2) {
				packetNum = currentChar;
			}
		}
	}
		if (isDone) {
			break;
		}

		// the read in data for the entire packet
		/*cout << "Values in Packet (" << packetBytes << " bytes_s): " << endl;
		for (int i = 0; i < packetBytes; i++) {
			char character = *(packetBuffer + i);
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;*/

		generatePacket();

		uint16_t checkSumValue = gen_crc16(packet2, payloadSize);
		cout << "Checksum of packet char*: " << checkSumValue << endl;

		//cout << "Read in checksum: " << (char)*(packetBuffer + packetBufferIndex - 2) << " and " << (char)*(packetBuffer + packetBufferIndex - 1) << endl;

		char chksum1 = (checkSumValue >> 8);
		char chksum2 = (checkSumValue & 0xFF);
		//cout << "Calculated checksum: " << chksum1 << endl;
		//cout << "Calculated checksum: " << chksum2 << endl;

		//checksum is good
		if (chksum1 == *(packetBuffer + packetBufferIndex - 2) && chksum2 == *(packetBuffer + packetBufferIndex - 1)) {
			//append to the file
			myfile << packet_s;

			/**** WRITING ****/
			bytes_s = 0;

			cout << "Packet " << packetNum << " received" << endl;

			numPacketsReceived++;
			sequenceNumber_s++;
			sequenceNumber_s %= seqNumRange;

			//send ack over to client with packet number
			char* ackMsg = &packetNum;

			if (!write(sockfd, ackMsg, sizeof(packetNum))) {
				cout << "2. ERROR writing to socket: " << sockfd << endl;
				break;
			}
			else {
				cout << "Ack " << packetNum << " sent" << endl;
			}
		}

		//printout of payload after taking out any DLE's
		/*	cout << "**** Values in Payload (" << adjustedPayloadSize << " bytes_s): " << endl;
		for (int i = 0; i < adjustedPayloadSize; i++) {
			cout << packet2[i];
		}
		cout << endl;*/

		//print out value of payload with * to represent our special characters and NUL
		/*cout << "**** Values in Payload (" << adjustedPayloadSize << " bytes_s): " << endl;
		for (int i = 0; i < adjustedPayloadSize; i++) {
			char character = packet2[i];
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;*/

		//boost checksum
		/*crcChecker.process_bytes(packet2, sizeof(packet2));
		cout << crcChecker.checksum() << endl;*/

		//cout << "To: thing2.cs.uwec.edu" << endl << endl;
	}

	myfile.close();

	double Uduration = (timer.GetTimeInMicroSeconds() - startTimeUSecs);

	cout.precision(5);
	cout << endl << "Packet Size Received: " << totalBytes_s << " bytes" << endl;
	cout << "Number of packets received: " << numPacketsReceived << endl;
	cout << "Total elapsed time: " << Uduration / 1000000 << "s" << endl;
	cout << "md5sum: " << "0" << endl;

	close(ss);
	close(sockfd);
}

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
int main(int argc, char **argv) {
	if (argc == 3) {
		int packetSizeInBytes = MAX_PACKET_SIZE;
		int seqNumberRange = MAX_SEQ_NUM_RANGE;
		char* ptr;
		unsigned long portNum = strtol(argv[1], &ptr, 10);
		if (portNum <= 9000 || portNum > 10000) {
			cerr << "ERROR with second parameter: " << argv[1] << ", it should be a valid port number between 9,000 and 10,000" << endl;
		}
		else {
			//if this process should be the server
			if (!strcmp(argv[2], "-s")) {
				server(portNum);
			}
			else if (!strcmp(argv[2], "-c")) {
				string fileName = GetFileName();
				
				int protocol = GetProtocol();
				int slidingWindowSize = 0;
				if (protocol == Protocol::GBN || protocol == Protocol::SR) {
					slidingWindowSize = GetSlidingWindowSize()
				}

				packetSizeInBytes = GetPacketSize();

				int intervalMethod = GetTimeOutIntervalMethod();
				int intervalTimeInMicroseconds = -1;
				if (intervalMethod == TOInterval::US) {
					intervalTimeInMicroseconds = GetTimeOutFromUser();
				}
				else {
					//use ping to pick a good timeout? or pass in -1 so that the client know that it needs to figure it out
				}

				seqNumberRange = GetSequenceNumberRange();

				int situationalErrorType = GetSituationalErrorType();
				if (situationalErrorType == SitErrors[SitError::US]) {
					int errorControlType = GetErrorControlType();
					//ack lost
					if (errorControlType == ErrorControls[ErrorControl::AL]) {
						
					}
					//packet damage
					else if (errorControlType == ErrorControls[ErrorControl::PD]) {

					}
					//packet loss
					else if (errorControlType == ErrorControls[ErrorControl::PL]) {

					}
					//multiple
					else {

					}
				}
				
				client(portNum, packetSizeInBytes, seqNumberRange, fileName);
			}
			else {
				cerr << "ERROR with third parameter: " << argv[2] << ", the proper flags are -c or -s" << endl;
			}
		}
	}
	else {
		cerr << "ERROR: Incorrect number of arguments. Command should be something like ./packet portNum -flag (s or c)" << endl;
	}

	exit(0);
}
