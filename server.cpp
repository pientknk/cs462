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

