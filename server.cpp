#include "server.h"
#include "printing.h"
#include <sys/poll.h>

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
int adjustedPayloadSize_s = 0;
int receivingWindowSize;
int largestAcceptableFrame;
int lastFrameReceived = -1;
int currentPacketNum = -1;
int expectedPacketNum = -1;

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
	adjustedPayloadSize_s = 0;
	for (int i = 3; i < payloadSize + 3; i++) {
		unsigned char packetChar = *(packetBuffer + i);
		if (packetChar == DLE) {
			packetChar = *(packetBuffer + i + 1);
			if (packetChar == DLE) {
				adjustedPayloadSize_s++;
				i++;
			}
		}
		else {
			adjustedPayloadSize_s++;
		}
	}

	//create string from buffer somehow
	packet_s = "";
	packet2 = new unsigned char[adjustedPayloadSize_s];
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

	//printPacketReplace(adjustedPayloadSize_s, packet2);
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
		ofstream myfile;
		myfile.open("Output.txt");
	
	isDone = false;
	while (!isDone) {
		/**** READING ****/
		foundEndOfPacket = false;
		packetBufferIndex = 0;
		packetBytes = 0;
		packetNum = 'Z';

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
				if (currentChar == ETX) {
					int z = 1;
					while(*(packetBuffer + packetBufferIndex - z) == DLE) {
						z++;
					}
					if(z % 2 == 1){
						foundEndOfPacket = true;

						//read in the checksum
						bytes_s = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
						packetBytes += bytes_s;
						packetBufferIndex += bytes_s;
					}
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

		generatePacket();

		packetNumInt = packetNum - '0';
		cout << "Expected seq#: " << sequenceNumber_s << endl;
		cout << "Packet " << packetNumInt << " received" << endl;
		numPacketsReceived++;

		uint16_t checkSumValue = gen_crc16(packet2, adjustedPayloadSize_s);

		unsigned char chksum1 = (checkSumValue >> 8);
		unsigned char chksum2 = (checkSumValue & 0xFF);

		//check for fake loss of an ack
		if (packetNumInt == -1) {
			shouldLoseAck = true;
		}
		else {
			shouldLoseAck = false;
		}
		
		if (packetNumInt == sequenceNumber_s || shouldLoseAck) {
			if (shouldLoseAck) {
				cout << "***** Losing ACK... ****" << endl;
				totalBytes_s += packetBytes;
				bytesDuplicate += packetBytes;
			}
			//checksum is good
			else if (chksum1 == *(packetBuffer + packetBufferIndex - 2) && chksum2 == *(packetBuffer + packetBufferIndex - 1)) {
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
			else {
				bytesDuplicate += packetBytes;
				cout << "Checksum for Packet " << packetNumInt << " has failed" << endl;
			}
		}
		else {
			bytesDuplicate += packetBytes;
			cout << "Unexpected Sequence number: " << packetNumInt << ".   Not sending ACK" << endl;
		}
	}	
	
	myfile.close();
	}
	else{
		if (protocol == "GBN") {	
			receivingWindowSize = 1;
			largestAcceptableFrame = receivingWindowSize - 1;
			lastFrameReceived = -1;
			currentPacketNum = -1;
			expectedPacketNum = -1;
			packetNum = 'Z';
			unsigned char* ackMsg;
			bool sendAcks = false;
			bool isDuplicate = true;
			
			struct pollfd pfd;
			pfd.fd = sockfd;
			pfd.events = POLLIN;
	
			ofstream myfile;
			myfile.open("Output.txt");
			
			isDone = false;
			bool isSuperDone = false;
			
			while (!isDone) {
				/**** READING ****/
				foundEndOfPacket = false;
				packetBufferIndex = 0;
				packetBytes = 0;

				while (!foundEndOfPacket) {
					int pollResult = poll(&pfd, 1, 100); 
					if(pollResult > 0){
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
							if (currentChar == ETX) {
								int z = 1;
								while(*(packetBuffer + packetBufferIndex - z) == DLE) {
									z++;
								}
								if(z % 2 == 1){
									foundEndOfPacket = true;

									//read in the checksum
									bytes_s = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
									packetBytes += bytes_s;
									packetBufferIndex += bytes_s;
								}
							}

							packetBufferIndex++;
							if (packetBytes == 2) {
								packetNum = currentChar;
							}
						}
					}
					else if(pollResult == 0){
						sendAcks = true;
						foundEndOfPacket = true;
					}
					else {
						isDone = true;
						break;
					}
				}
				
				if (isDone) {
					break;
				}
				
				packetNumInt = packetNum - '0';
				if(lastFrameReceived < packetNumInt && packetNumInt <= largestAcceptableFrame) {
					generatePacket();

					cout << "Expected seq#: " << sequenceNumber_s << endl;
					cout << "Packet " << packetNumInt << " received" << endl;
					numPacketsReceived++;

					uint16_t checkSumValue = gen_crc16(packet2, adjustedPayloadSize_s);

					unsigned char chksum1 = (checkSumValue >> 8);
					unsigned char chksum2 = (checkSumValue & 0xFF);

					//check for fake loss of an ack
					if (packetNumInt == -1) {
						shouldLoseAck = true;
					}
					else {
						shouldLoseAck = false;
					}
					
					if (packetNumInt == sequenceNumber_s || shouldLoseAck) {
						if (shouldLoseAck) {
							cout << "***** Losing ACK... ****" << endl;
							totalBytes_s += packetBytes;
							bytesDuplicate += packetBytes;
						}
						//checksum is good
						else if (chksum1 == *(packetBuffer + packetBufferIndex - 2) && chksum2 == *(packetBuffer + packetBufferIndex - 1)) {
							cout << "Checksum for Packet " << packetNumInt << " was OK" << endl;

							bytes_s = 0;

							if (packetNum != lastPacketNum) {
								lastPacketNum = packetNum;
								bytesReceived += packetBytes;
								lastFrameReceived = packetNumInt;
								//append to the file	
								myfile << packet_s;
							}
							//send ack over to client with packet number
							ackMsg = &packetNum;
							
							sequenceNumber_s++;
							largestAcceptableFrame++;
							totalBytes_s += packetBytes;
						}
						else {
							bytesDuplicate += packetBytes;
							cout << "Checksum for Packet " << packetNumInt << " has failed" << endl;
						}
						
						cout << "Current window = [" << packetNumInt + 1 << "]" << endl;
					}
					else {
						bytesDuplicate += packetBytes;
						cout << "Unexpected Sequence number: " << packetNumInt << ".   Not sending ACK" << endl;
						cout << "Current window = [" << packetNumInt << "]" << endl;
					}
				}	
				
				/**** WRITING ****/
				if(sendAcks) {
					int bytes_w = write(sockfd, ackMsg, 1);
					if (!bytes_w) {
						cout << "2. ERROR writing to socket: " << sockfd << endl;
					}
					else {
						cout << "Ack " << packetNumInt << " sent" << endl;
						sendAcks = false;
					}
				}
				
				if(packetNumInt == 207 && isDuplicate) {
					sequenceNumber_s = 0;
					packetNumInt = 0;
					lastFrameReceived -= 208;
					largestAcceptableFrame -= 208;
					isDuplicate = false;
				}
				else {
					isDuplicate = true;
				}
			}
			
			myfile.close();
		}
		//selective repeat
		else {
			cout << "Selective repeat" << endl;
		}
	}

	cout << "Session successfully terminated" << endl;

	double Uduration = (timer.GetCurrentTimeInMicroSeconds() - startTimeUSecs);
	int lastPacketNumInt = lastPacketNum - '0';

	printServerStats(lastPacketNumInt, bytesReceived, bytesDuplicate, numPacketsReceived, Uduration);

	close(ss);
	close(sockfd);
}