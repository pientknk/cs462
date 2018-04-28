#include "server.h"
#include "printing.h"

int sockfd;
string packet_s;
string fileContents;
unsigned char* packet2;
unsigned char packetNum;
int numPacketsReceived = 0;
int packetNumInt;
unsigned char lastPacketNum = USCR;
int lastPacketNumInt = 0;
int packetBytes;
unsigned char* packetBuffer;
int packetBufferIndex;
int payloadSize;
int sequenceNumber_s;
int bytes_s;
int totalBytes_s;
int bytesReceived = 0;
int bytesDuplicate = 0;
bool foundEndOfPacket;
bool isDone;
int adjustedPayloadSize = 0;

//Error introduction variables
bool failToSendAcks = false;
bool shouldLoseAck = false;
int i;

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
	cout << endl;
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
			unsigned char currentChar = *(packetBuffer + packetBufferIndex);
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
	adjustedPayloadSize = 0;
	for (int i = 3; i < payloadSize + 3; i++) {
		unsigned char packetChar = *(packetBuffer + i);
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
	packet_s = "";
	packet2 = new unsigned char[adjustedPayloadSize];
	int packet2Index = 0;
	for (int i = 3; i < payloadSize + 3; i++) {
		unsigned char packetChar = *(packetBuffer + i);
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

	//cout << "Packet size: " << adjustedPayloadSize << endl;
}

void server(int portNum)
{
	sequenceNumber_s = 0;
	numPacketsReceived = 0;
	totalBytes_s = 0;
	bytes_s = 0;
	packetBytes = 0;

	int ss = serverSocketSetup(portNum);
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);

	//Time variables
	Timer timer;
	suseconds_t startTimeUSecs = timer.GetCurrentTimeInMicroSeconds();
	double duration = 0.0;

	//Buffer
	packetBuffer = new unsigned char[int(MAX_PACKET_SIZE * 1.5) + PACKET_FRAME_SIZE];

	//read in the sequence number range
	int seqNumRange = 0;
	int* seqNumRangePointer = &seqNumRange;
	if (!read(sockfd, seqNumRangePointer, sizeof(int))) {
		//cout << "1. ERROR reading from socket: " << sockfd << endl;
	}
	
	//read in the protocol to expected
	char* protocolInfo = new char[2];
	
	if (!read(sockfd, protocolInfo, sizeof(protocolInfo))) {
		//cout << "1. ERROR reading from socket: " << sockfd << endl;
	}
	
	string protocol(protocolInfo);
	
	if (protocol == "SW") {
		//serverStopAndWait(portNum, ss, seqNumRange);
		
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
				//cout << "1. ERROR reading from socket: " << sockfd << endl;
				isDone = true;
				
				cout << "bytes_s: " << bytes_s << endl;
				cout << "error: " << strerror(errno) << endl;
				break;
			}
			else {
				packetBytes += bytes_s;

				//get current char to check if this is the end of the packet
				unsigned char currentChar = *(packetBuffer + packetBufferIndex);
				if (currentChar == ETX && *(packetBuffer + packetBufferIndex - 1) != DLE) {
					foundEndOfPacket = true;

					//read in the checksum
					bytes_s = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
					packetBytes += bytes_s;
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

		//printPacketReplace(packetBytes, packetBuffer);

		generatePacket();

		/*string contents = "";

		for (int i = 0; i < adjustedPayloadSize; i++) {
			unsigned char character = *(packet2 + i);
			contents += character;
		}

		ofstream afile;
		afile.open("ServerOutput.txt");
		afile << contents;
		afile.close();
		cout << "Wrote file ServerOutput.txt" << endl;*/

		packetNumInt = packetNum - '0';

		cout << "Packet " << packetNumInt << " received" << endl;
		numPacketsReceived++;

		uint16_t checkSumValue = gen_crc16(packet2, adjustedPayloadSize);
		//printCheckSum(checkSumValue);
		//printCheckSumIndividual(checkSumValue);
		//cout << "Read in checksum: " << (char)*(packetBuffer + packetBufferIndex - 2) << " and " << (char)*(packetBuffer + packetBufferIndex - 1) << endl;

		unsigned char chksum1 = (checkSumValue >> 8);
		unsigned char chksum2 = (checkSumValue & 0xFF);

		//check for fake loss of an ack
		if (packetNumInt == MAX_SEQ_NUM_RANGE + 1) {
			shouldLoseAck = true;
		}
		else {
			shouldLoseAck = false;
		}
		
		if (packetNumInt == sequenceNumber_s || shouldLoseAck) {
			//checksum is good
			if (chksum1 == *(packetBuffer + packetBufferIndex - 2) && chksum2 == *(packetBuffer + packetBufferIndex - 1)) {
				cout << "Checksum for Packet " << packetNumInt << " was OK" << endl;

				/**** WRITING ****/
				bytes_s = 0;

				if (packetNum != lastPacketNum) {
					lastPacketNum = packetNum;
					bytesReceived += packetBytes;
					//append to the file
					myfile << packet_s;
				}

				//send ack over to client with packet number
				unsigned char* ackMsg = &packetNum;

				if (shouldLoseAck) {
					cout << "Losing ACK..." << endl;
					sequenceNumber_s++;
					sequenceNumber_s %= seqNumRange;
					totalBytes_s += packetBytes;
					bytesDuplicate += packetBytes;
				}
				else {
					if (!write(sockfd, ackMsg, sizeof(packetNum))) {
						cout << "2. ERROR writing to socket: " << sockfd << endl;
						break;
					}
					else {
						cout << "Ack " << packetNumInt << " sent" << endl;
						sequenceNumber_s++;
						sequenceNumber_s %= seqNumRange;

						totalBytes_s += packetBytes;
					}
				}
			}
			else {
				bytesDuplicate += packetBytes;
				cout << "Checksum for Packet " << packetNumInt << " has failed" << endl;
			}
		}
		else {
			bytesDuplicate += packetBytes;
			cout << "Unexpected Sequence number: " << packetNumInt << ".   Not sending ACK" << endl;
		}


		//printout of payload after taking out any DLE's
		//printPayload(adjustedPayloadSize, packet2);

		//print out value of payload with * to represent our special characters and NUL
		//printPayloadReplace(adjustedPayloadSize, packet2);
	}	
	
	myfile.close();
	}
	else{
		if (protocol == "GBN") {
			//serverGBN(seqNumRange);
		}
		//selective repeat
		else {
	
		}
	}

	cout << "Session successfully terminated" << endl;

	double Uduration = (timer.GetCurrentTimeInMicroSeconds() - startTimeUSecs);
	int lastPacketNumInt = lastPacketNum - '0';

	printServerStats(lastPacketNumInt, bytesReceived, bytesDuplicate, numPacketsReceived, Uduration);

	close(ss);
	close(sockfd);
}

void serverStopAndWait(int portNum, int ss, int seqNumRange) {
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
				//cout << "1. ERROR reading from socket: " << sockfd << endl;
				isDone = true;

				break;
			}
			else {
				packetBytes += bytes_s;

				//get current char to check if this is the end of the packet
				unsigned char currentChar = *(packetBuffer + packetBufferIndex);
				if (currentChar == ETX && *(packetBuffer + packetBufferIndex - 1) != DLE) {
					foundEndOfPacket = true;

					//read in the checksum
					bytes_s = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
					packetBytes += bytes_s;
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

		//printPacketReplace(packetBytes, packetBuffer);

		generatePacket();

		/*string contents = "";

		for (int i = 0; i < adjustedPayloadSize; i++) {
			unsigned char character = *(packet2 + i);
			contents += character;
		}

		ofstream afile;
		afile.open("ServerOutput.txt");
		afile << contents;
		afile.close();
		cout << "Wrote file ServerOutput.txt" << endl;*/

		packetNumInt = packetNum - '0';

		cout << "Packet " << packetNumInt << " received" << endl;
		numPacketsReceived++;

		uint16_t checkSumValue = gen_crc16(packet2, adjustedPayloadSize);
		//printCheckSum(checkSumValue);
		//printCheckSumIndividual(checkSumValue);
		//cout << "Read in checksum: " << (char)*(packetBuffer + packetBufferIndex - 2) << " and " << (char)*(packetBuffer + packetBufferIndex - 1) << endl;

		unsigned char chksum1 = (checkSumValue >> 8);
		unsigned char chksum2 = (checkSumValue & 0xFF);

		//check for fake loss of an ack
		if (packetNumInt == MAX_SEQ_NUM_RANGE + 1) {
			shouldLoseAck = true;
		}
		else {
			shouldLoseAck = false;
		}
		
		if (packetNumInt == sequenceNumber_s || shouldLoseAck) {
			//checksum is good
			if (chksum1 == *(packetBuffer + packetBufferIndex - 2) && chksum2 == *(packetBuffer + packetBufferIndex - 1)) {
				cout << "Checksum for Packet " << packetNumInt << " was OK" << endl;

				/**** WRITING ****/
				bytes_s = 0;

				if (packetNum != lastPacketNum) {
					lastPacketNum = packetNum;
					bytesReceived += packetBytes;
					//append to the file
					myfile << packet_s;
				}

				//send ack over to client with packet number
				unsigned char* ackMsg = &packetNum;

				if (shouldLoseAck) {
					cout << "Losing ACK..." << endl;
					sequenceNumber_s++;
					sequenceNumber_s %= seqNumRange;
					totalBytes_s += packetBytes;
					bytesDuplicate += packetBytes;
				}
				else {
					if (!write(sockfd, ackMsg, sizeof(packetNum))) {
						cout << "2. ERROR writing to socket: " << sockfd << endl;
						break;
					}
					else {
						cout << "Ack " << packetNumInt << " sent" << endl;
						sequenceNumber_s++;
						sequenceNumber_s %= seqNumRange;

						totalBytes_s += packetBytes;
					}
				}
			}
			else {
				bytesDuplicate += packetBytes;
				cout << "Checksum for Packet " << packetNumInt << " has failed" << endl;
			}
		}
		else {
			bytesDuplicate += packetBytes;
			cout << "Unexpected Sequence number: " << packetNumInt << ".   Not sending ACK" << endl;
		}


		//printout of payload after taking out any DLE's
		//printPayload(adjustedPayloadSize, packet2);

		//print out value of payload with * to represent our special characters and NUL
		//printPayloadReplace(adjustedPayloadSize, packet2);
	}	
	
	myfile.close();
}