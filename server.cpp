#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <limits.h>
#include <ctime>
#include <sstream>
#include <fstream>
#include <sys/time.h>

#include <boost/crc.hpp>

using namespace std;
const char SOH = 0x01; //Start of Header
const char STX = 0x02; //Start of Text
const char ETX = 0x03; //End of Text
const char EOP = 0x7E; //End of Packet
const char DLE = 0x10; //Data Link Escape
const char ACK = 0x06; //ACK Ascii character, also (0000 0110)
const int MAX_PACKET_SIZE = 2048; //payload size
const int PACKET_FRAME_SIZE = 6; //the amount of bytes needed to frame the packet
const int MAX_SEQ_NUM_RANGE = CHAR_MAX;

//initialize socket for the server
int serverSocketSetup(int portNum){
	//get the right socket file descriptor
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
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
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("ERROR with binding\n");
		exit(1);
	}
	
	return sockfd;
}

int callServer(string host, int portNum){
	const char* constHost = host.c_str();
	//socket pointer
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		cerr << "ERROR opening socket" << endl;
		exit(0);
	}
	
	//server address structure
	struct sockaddr_in serv_addr;
	
	//set all the values in the server address to 0
	memset(&serv_addr, '0', sizeof(serv_addr));
	
	//setup the type of socket to be internet
	serv_addr.sin_family = AF_INET;
	
	//setup the port number
	serv_addr.sin_port = htons(portNum);
	
	//setup the server host address
	struct hostent *server;
	server = gethostbyname(constHost);
	if(server == NULL){
		cerr << "Error, host " << constHost << " was not found" << endl;
		exit(0);
	}
	
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length); //destination, source, sizeof
	
	//connect to the server
	if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		cerr << "ERROR connecting to the server" << endl;
		exit(0);
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
	if(sockfd < 0){
		cerr << "ERROR accepting connection" << endl;
		exit(1);
	}
	
	return sockfd;
}

//return a string of all characters in the designated file
char* parseFile(string fileName, long fileSize) {
	FILE *filePtr;
	filePtr = fopen(fileName.c_str(), "rb");
	if (filePtr == NULL) {
		cerr << "ERROR opening file: " << fileName << endl;
		return "-1";
	}
	else {
		char *buffer = (char*)malloc(sizeof(char)*fileSize);
		if (buffer == NULL) {
			cerr << "Memory Error" << endl;
			return "-1";
		}
		else {
			size_t result = fread(buffer, 1, fileSize, filePtr);
			if (result != fileSize) {
				cerr << "Reading File Error" << endl;
				return "-1";
			}
			else {
				return buffer;
			}
		}
	}


	/*ifstream myfile(fileName);
	if (myfile.good() && myfile.is_open()) {
		string input((istreambuf_iterator<char>(myfile)), istreambuf_iterator<char>());
		return input;
	}
	else {
		cout << "ERROR: Could not open file" << endl;
		return "-1";
	}*/


}

long getFileSize(string fileName) {
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
 
void server(int portNum)
{
	int sequenceNumber = 0;
	int numPacketsReceived = 0;
	int totalBytes = 0;
	int bytes = 0;
	int packetBytes = 0;
	
	boost::crc_basic<16>  crcChecker( 0x1021, 0xFFFF, 0, false, false );
	short crcValue;
	
	int ss = serverSocketSetup(portNum);
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);

	//Time variables
	struct timeval time;
	gettimeofday(&time, NULL);
	time_t startTime = time.tv_sec;
	suseconds_t startTimeUSecs = time.tv_usec;
	double duration = 0.0;

	//Buffer
	char* packetBuffer = new char[int(MAX_PACKET_SIZE * 1.5) + PACKET_FRAME_SIZE];
	char* checksumString = new char[2];
	
	//read in the sequence number range
	int seqNumRange = 0;
	int* seqNumRangePointer = &seqNumRange;
	if (!read(sockfd, seqNumRangePointer, sizeof(int))) {
		cout << "1. ERROR reading from socket: " << sockfd << endl;
	}

	ofstream myfile;
	myfile.open("Output.txt");

	bool isDone = false;
	while(!isDone){
		/**** READING ****/
		cout << "Expected seq#: " << sequenceNumber << endl;
		bool foundEndOfPacket = false;
		int packetBufferIndex = 0;
		packetBytes = 0;
		while (!foundEndOfPacket) {
			bytes = read(sockfd, packetBuffer + packetBufferIndex, 1); //if slow, read in as much as we can and loop through the buffer to see if there is an ETX
			if (bytes <= 0) {
				//cout << "1. ERROR reading from socket: " << sockfd << endl;
				isDone = true;
				break;
			}
			else {
				//get current char to check if this is the end of the packet
				char currentChar = *(packetBuffer + packetBufferIndex);
				if (packetBufferIndex != 0 && currentChar == ETX) {
					foundEndOfPacket = true;
				}

				packetBytes += bytes;
				totalBytes += bytes;
				packetBufferIndex++;
				
				if(foundEndOfPacket) {
					bytes = read(sockfd, packetBuffer + packetBufferIndex, 2);
					if(bytes <= 0 || bytes != 2) {
						cout << "1. ERROR reading from socket: " << sockfd << endl;
						isDone = true;
						break;
					}
				}
			}
		}

		if (isDone) {
			break;
		}

		//create string from buffer somehow
		string packet;
		for (int i = 0; i < packetBytes; i++) {
			char packetChar = *(packetBuffer + i);
			if(!(packetChar == SOH || packetChar == STX || packetChar == ETX || packetChar == DLE)){
				packet += packetChar;
			}
		}
		
		cout << "Checksum value: " << checksumString << endl;

		packet.erase(0, 1);

		cout << "Packet string: " << packet << endl;
		myfile << packet;

		/*cout << "Server got value: " << endl;
		for (int i = 0; i < packetSize+3; i++) {
		cout << *(packetBuffer + i) << "";
		}
		cout << endl;*/

		//bytes = read(sockfd, packetBuffer, packetSize + PACKET_FRAME_SIZE);
		//if(bytes <= 0){
		//	cout << "1. ERROR reading from socket: " << sockfd << endl;
		//	break;
		//}
		//else {
		//	totalBytes += bytes;
		//	numPacketsReceived++;
		//	/*cout << "Server got value: " << endl;
		//	for (int i = 0; i < bytes; i++) {
		//		cout << *(packetBuffer + i) << "";
		//	}
		//	cout << endl;*/
		//}
		
		/**** WRITING ****/
		bytes = 0;
		char packetNum = 'Z';

		for (int i = 0; i < packetBytes; i++) {
			if (i == 1) {
				packetNum = *(packetBuffer + i);
			}
			else if (i > 1) {
				break;
			}
		}

		cout << "Packet " << packetNum << " received" << endl;

		numPacketsReceived++;
		sequenceNumber++;
		sequenceNumber %= seqNumRange; 

		//send ack over to client with packet number
		char* ackMsg = &packetNum;
		
		if(!write(sockfd, ackMsg, sizeof(packetNum))){
			cout << "2. ERROR writing to socket: " << sockfd << endl;
			break;
		} 
		else {
			cout << "Ack " << packetNum << " sent" << endl;
		}
		
		//cout << "To: thing2.cs.uwec.edu" << endl << endl;
	}

	myfile.close();

	gettimeofday(&time, NULL);
	double Uduration = (time.tv_usec - startTimeUSecs);
	
	cout.precision(5);
	cout << endl << "Packet Size Received: " << totalBytes << " bytes" << endl;
	cout << "Number of packets received: " << numPacketsReceived << endl;
	cout << "Total elapsed time: " << Uduration / 1000000 << "s" << endl;
	cout << "md5sum: " << "0" << endl;

	close(ss);
	close(sockfd);
}

void client(int portNum, int packetSize, int seqNumberRange, string fileName)
{
	int socket = callServer("thing3.cs.uwec.edu", portNum);
	int bytesWritten = 0;
	int numPacketsSent = 0;
	char ack;
	char* ackBuffer = &ack;
	int sequenceNumber = 0;
	int currentIndex = 0;
	
	boost::crc_basic<16>  crcChecker( 0x1021, 0xFFFF, 0, false, false );

	//get file data and setup packet
	long totalBytes = getFileSize(fileName);
	char* payload = parseFile(fileName, totalBytes);

	char packet[int(MAX_PACKET_SIZE * 1.5)];

	//Time variables
	struct timeval time;
	gettimeofday(&time, NULL);
	time_t startTime = time.tv_sec;
	suseconds_t startTimeUSecs = time.tv_usec;
	double duration = 0.0;

	//Send over Packet Sequence Number Range information
	int* packetSizePointer = &seqNumberRange;
	int bytes = 0;
	if (!write(socket, packetSizePointer, sizeof(int))) {
		cout << "1. ERROR reading from socket: " << socket << endl;
	}

	//buffer
	char* packetBuffer = new char[int(packetSize * 1.5)];
	for (int i = 0; i < packetSize; i++) {
		*(packetBuffer + i) = '0';
	}

	while(totalBytes){
		int currentPacketSize = packetSize;
		if (totalBytes < packetSize) {
			currentPacketSize = totalBytes;
		}

		/**** WRITING ****/
		char* packetPayloadPointer = payload + currentIndex;
		currentIndex += currentPacketSize;
		char* packetPayload = new char[currentPacketSize];
		strncpy(packetPayload, packetPayloadPointer, currentPacketSize);
		crcChecker.process_bytes(packetPayload, sizeof(packetPayload) / sizeof(packetPayload[0]));
		//cout << "Checksum: " << crcChecker.checksum() << endl;

		/*for (int i = 0; i < currentPacketSize; i++) {
			cout << *(packetPayload + i);
		}
		cout << endl;*/

		//packet vars
		char packetNum = '0' + sequenceNumber;
		int packetIndex = 0;
		int packetPayloadIndex = 0;

		//asssign header of packet
		for (int i = 0; i <= 2; i++) {
			if (i == 0) {
				packet[i] = SOH;
			}
			else if (i == 1) {
				packet[i] = packetNum;
			}
			else {
				packet[i] = STX;
			}
		}

		//assign payload of packet
		int i = PACKET_FRAME_SIZE - 1;
		while (packetPayloadIndex != currentPacketSize) {
			char currentPayloadChar = packetPayload[packetPayloadIndex];
			if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX) {
				packet[i] = DLE;
				i++;
			}
			packet[i] = currentPayloadChar;
			i++;
			packetPayloadIndex++;
		}

		//assign end of header
		packet[i] = ETX;
		
		short checksum = crcChecker.checksum();
		
		packet[i + 1] = (checksum >> 8);
		packet[i + 2] = (checksum & 0xFF);

		/*	while(packetIndex != currentPacketSize + PACKET_FRAME_SIZE) {
			if (packetIndex == 0) {
				packet[packetIndex] = SOH;
			}
			else if (packetIndex == 1) {
				packet[packetIndex] = packetNum;
			}
			else if (packetIndex == 2) {
				packet[packetIndex] = STX;
			}
			else if(packetIndex == currentPacketSize + PACKET_FRAME_SIZE - 1){
				packet[packetIndex] = ETX;
			}
			else {
				char currentPayloadChar = packetPayload[packetPayloadIndex];
				if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX) {
					packet[packetIndex] = DLE;
					packetIndex++;
				}
				packet[packetIndex] = packetPayload[packetPayloadIndex];
				packetPayloadIndex++;
			}
			packetIndex++;
		}*/

		cout << "Values in packet: " << endl;
		for (int i = 0; i < currentPacketSize + PACKET_FRAME_SIZE; i++) {
			cout << packet[i] << "";
		}
		cout << endl;

		//get updated time
		gettimeofday(&time, NULL);
		suseconds_t startWriteUSec = time.tv_usec;

		packetBuffer = packet;
		bytes = write(socket, packetBuffer, currentPacketSize + PACKET_FRAME_SIZE);
		if(bytes <= 0)
		{
			cout << "1. ERROR writing to socket: " << socket << endl;
			break;
		}
		else {
			totalBytes -= (bytes - PACKET_FRAME_SIZE);
			bytesWritten += bytes;
			numPacketsSent++;
			/*cout << "Client wrote value: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(packetBuffer + i) << "";
			}
			cout << endl;*/

			cout << "Packet " << sequenceNumber << " sent" << endl;
		}
		
		/**** READING ****/
		bytes = 0;
		bytes = read(socket, ackBuffer, sizeof(ack));
		if(bytes <= 0){
			cout << "2. ERROR reading from socket: " << socket << endl;
			break;
		}
		else {
			/*cout << "Client got ACK: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(ackBuffer + i) << "";
			}
			cout << endl;*/

			//Get update time
			gettimeofday(&time, NULL);
			suseconds_t endWriteUSec = time.tv_usec;
			double rtt = endWriteUSec - startWriteUSec;

			cout << "Ack " << sequenceNumber << " received. (RTT for pkt " << sequenceNumber << " = " << rtt << "us)" << endl;
		}

		sequenceNumber++;
		sequenceNumber %= seqNumberRange;
	}

	// get updated time
	gettimeofday(&time, NULL);
	double Uduration = (time.tv_usec - startTimeUSecs);
	double throughput = ((double)bytesWritten / (double)Uduration);

	//print stats
	cout.precision(5);
	cout << endl << "Packet Size: " << bytesWritten << " bytes" << endl;
	cout << "Number of packets sent: " << numPacketsSent << endl;
	cout << "Total elapsed time: " << Uduration / 1000000 << "s" << endl;
	cout << "Throughput (Mbps): " << throughput << endl;
	cout << "md5sum: " << "0" << endl;

	close(socket);

	//cout << endl << "To: thing3.cs.uwec.edu" << endl << endl;
}


//TO RUN SERVER: ./packet 9036 -s
//TO RUN CLIENT: ./packet 9036 -c
int main(int argc, char **argv){
	if(argc == 3){
		int packetSizeInBytes = MAX_PACKET_SIZE;
		int seqNumberRange = MAX_SEQ_NUM_RANGE;
		char* ptr;
		// Max limit cannot be more than 2,147,483,647
		unsigned long portNum = strtol(argv[1], &ptr, 10); //returns 0 if argv[1] cannot be converted to an integer
		if (portNum <= 9000 || portNum > 10000) {
			cerr << "ERROR with second parameter: " << argv[1] << ", it should be a valid port number between 9,000 and 10,000" << endl;
		} else{
			//if this process should be the server
			if(!strcmp(argv[2], "-s")){
				server(portNum);
			} else if(!strcmp(argv[2], "-c")){
				//GET the file to send
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
				size_t* st;
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
						cout << "\nERROR: Packet size must be between 1 and " << MAX_PACKET_SIZE - PACKET_FRAME_SIZE << " bytes, but you said: '" << tempPacketSize << "'" << endl;
					}
				}

				// GET range of sequence numbers from user
				bool choseSeqNumRange = false;
				string seqNumRange;
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
				client(portNum, packetSizeInBytes, seqNumberRange, fileName);
			} else{
				cerr << "ERROR with third parameter: " << argv[2] << ", the proper flags are -c or -s" << endl;
			}
		}
	}
	else{
		cerr << "ERROR: Incorrect number of arguments. Command should be something like ./packet portNum -flag (s or c)" << endl;
	}

	exit(0);
}

