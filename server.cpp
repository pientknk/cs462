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
const char NUL = 0x00; //null ascii value
const char SOH = 0x01; //Start of Header
const char STX = 0x02; //Start of Text
const char ETX = 0x03; //End of Text
const char EOP = 0x7E; //End of Packet
const char DLE = 0x10; //Data Link Escape
const char ACK = 0x06; //ACK Ascii character, also (0000 0110)
const int MAX_PACKET_SIZE = 2048; //payload size
const int PACKET_FRAME_SIZE = 6; //the amount of bytes needed to frame the packet
const int MAX_SEQ_NUM_RANGE = CHAR_MAX;

typedef struct Packet {
	int SOHIndex = 0;
	int seqNumIndex = 1;
	int SeqNum;
	int STXIndex = 2;
	const char* data;
	int PayloadIndex = 3;
	suseconds_t startTimeUSecs;
} Packet;

typedef struct Timer {
	struct timeval time;
	time_t GetTimeInSeconds() {
		gettimeofday(&time, NULL);
		return time.tv_sec;
	}
	suseconds_t GetTimeInMicroSeconds() {
		gettimeofday(&time, NULL);
		return time.tv_usec;
	}
} Timer;

//initialize socket for the server
int serverSocketSetup(int portNum) {
	//get the right socket file descriptor
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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

int callServer(string host, int portNum) {
	const char* constHost = host.c_str();
	//socket pointer
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
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
	if (server == NULL) {
		cerr << "Error, host " << constHost << " was not found" << endl;
		exit(0);
	}

	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length); //destination, source, sizeof

	//connect to the server
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
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
	if (sockfd < 0) {
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

#define CRC16 0x8005

uint16_t gen_crc16(const char *data, uint16_t size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if (data == NULL)
		return 0;

	while (size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

		/* Increment bit counter: */
		bits_read++;
		if (bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		/* Cycle check: */
		if (bit_flag)
			out ^= CRC16;

	}

	// item b) "push out" the last 16 bits
	int i;
	for (i = 0; i < 16; ++i) {
		bit_flag = out >> 15;
		out <<= 1;
		if (bit_flag)
			out ^= CRC16;
	}

	// item c) reverse the bits
	uint16_t crc = 0;
	i = 0x8000;
	int j = 0x0001;
	for (; i != 0; i >>= 1, j <<= 1) {
		if (i & out) crc |= j;
	}

	return crc;
}

void server(int portNum)
{
	int sequenceNumber = 0;
	int numPacketsReceived = 0;
	int totalBytes = 0;
	int bytes = 0;
	int packetBytes = 0;

	boost::crc_basic<16>  crcChecker(0x1021, 0xFFFF, 0, false, false);
	short crcValue;

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
	char* packetBuffer = new char[int(MAX_PACKET_SIZE * 1.5) + PACKET_FRAME_SIZE];

	//read in the sequence number range
	int seqNumRange = 0;
	int* seqNumRangePointer = &seqNumRange;
	if (!read(sockfd, seqNumRangePointer, sizeof(int))) {
		cout << "1. ERROR reading from socket: " << sockfd << endl;
	}

	ofstream myfile;
	myfile.open("Output.txt");

	bool isDone = false;
	while (!isDone) {
		/**** READING ****/
		cout << "Expected seq#: " << sequenceNumber << endl;
		bool foundEndOfPacket = false;
		int packetBufferIndex = 0;
		packetBytes = 0;
		char packetNum = 'Z';

		while (!foundEndOfPacket) {
			bytes = read(sockfd, packetBuffer + packetBufferIndex, 1); //if slow, read in as much as we can and loop through the buffer to see if there is an ETX
			if (bytes <= 0) {
				//cout << "1. ERROR reading from socket: " << sockfd << endl;
				isDone = true;
				break;
			}
			else {
				packetBytes += bytes;
				totalBytes += bytes;


				//get current char to check if this is the end of the packet
				char currentChar = *(packetBuffer + packetBufferIndex);
				if (currentChar == ETX && *(packetBuffer + packetBufferIndex - 1) != DLE) {
					foundEndOfPacket = true;

					//read in the checksum
					bytes = read(sockfd, packetBuffer + packetBufferIndex + 1, 2);
					packetBytes += bytes;
					totalBytes += bytes;
					packetBufferIndex += bytes;
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

		cout << "Values in Packet (" << packetBytes << " bytes): " << endl;
		for (int i = 0; i < packetBytes; i++) {
			char character = *(packetBuffer + i);
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;

		/*cout << "Modified Values in Packet" << endl;
		int actualPacketSize = 0;
		for (int i = 3; i < packetBytes - 1; i++) {
			char character = *(packetBuffer + i);
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
				actualPacketSize++;
			}
		}
		cout << endl;*/

		//create string from buffer somehow
		string packet;
		int payloadSize = packetBytes - PACKET_FRAME_SIZE;
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

		char* packet2 = new char[adjustedPayloadSize];
		int packet2Index = 0;
		for (int i = 3; i < payloadSize + 3; i++) {
			char packetChar = *(packetBuffer + i);
			if (packetChar == DLE) {
				i++;
				packetChar = *(packetBuffer + i);
				packet += packetChar;
				packet2[packet2Index] = packetChar;
				packet2Index++;
			}
			else if (!(packetChar == SOH || packetChar == STX || packetChar == ETX)) {
				packet += packetChar;
				packet2[packet2Index] = packetChar;
				packet2Index++;
			}
		}

		//cout << "Packet 2 index: " << packet2Index << endl;

		//cout << "Packet string (" << sizeof(packet) << " bytes): " << endl << packet << endl;

		uint16_t value = gen_crc16(packet.c_str(), payloadSize);
		cout << "Checksum of packet string: " << value << endl;
		uint16_t value2 = gen_crc16(packet2, payloadSize);

	/*	cout << "**** Values in Payload (" << adjustedPayloadSize << " bytes): " << endl;
		for (int i = 0; i < adjustedPayloadSize; i++) {
			cout << packet2[i];
		}
		cout << endl;*/

		cout << "**** Values in Payload (" << adjustedPayloadSize << " bytes): " << endl;
		for (int i = 0; i < adjustedPayloadSize; i++) {
			char character = packet2[i];
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;

		cout << "Checksum of packet char*: " << value2 << endl;

		//crcChecker.process_bytes(packet2, sizeof(packet2));
		//cout << crcChecker.checksum() << endl;

		//append to the file
		myfile << packet;

		/**** WRITING ****/
		bytes = 0;

		cout << "Packet " << packetNum << " received" << endl;

		numPacketsReceived++;
		sequenceNumber++;
		sequenceNumber %= seqNumRange;

		//send ack over to client with packet number
		char* ackMsg = &packetNum;

		if (!write(sockfd, ackMsg, sizeof(packetNum))) {
			cout << "2. ERROR writing to socket: " << sockfd << endl;
			break;
		}
		else {
			cout << "Ack " << packetNum << " sent" << endl;
		}

		//cout << "To: thing2.cs.uwec.edu" << endl << endl;
	}

	myfile.close();

	double Uduration = (timer.GetTimeInMicroSeconds() - startTimeUSecs);

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

	boost::crc_basic<16>  crcChecker(0x1021, 0xFFFF, 0, false, false);

	//get file data and setup packet
	long totalBytes = getFileSize(fileName);
	char* payload = parseFile(fileName, totalBytes);

	//print out file contents
	/*cout << "Value of File: " << endl;
	for (int i = 0; i < totalBytes; i++) {
		char character = *(payload + i);
		if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
			cout << "*";
		}
		else {
			cout << character;
		}
	}
	cout << endl;*/

	char* packet;

	//Time variables
	Timer timer;
	time_t startTime = timer.GetTimeInSeconds();
	suseconds_t startTimeUSecs = timer.GetTimeInMicroSeconds();
	double duration = 0.0;

	//Send over Packet Sequence Number Range information
	int* packetSizePointer = &seqNumberRange;
	int bytes = 0;
	if (!write(socket, packetSizePointer, sizeof(int))) {
		cout << "1. ERROR reading from socket: " << socket << endl;
	}

	while (totalBytes) {
		int currentPacketSize = packetSize;
		if (totalBytes < packetSize) {
			currentPacketSize = totalBytes;
		}

		/**** WRITING ****/
		char* packetPayloadPointer = payload + currentIndex;
		currentIndex += currentPacketSize;
		char* packetPayload = new char[currentPacketSize];
		strncpy(packetPayload, packetPayloadPointer, currentPacketSize);

		//print off packet payload after copying it over from the file
		/*cout << "Values in Packet Payload: " << endl;
		for (int i = 0; i < currentPacketSize; i++) {
			char character = *(packetPayload + i);
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;*/

		//boost checksum and displaying it along with payload contents
		/*crcChecker.process_bytes(packetPayload, sizeof(packetPayload));
		cout << "Checksum: " << crcChecker.checksum() << endl;
		crcChecker.process_bytes(packetPayload, sizeof(packetPayload));
		cout << "Checksum2: " << crcChecker.checksum() << endl;

		cout << "Values in Payload: " << endl;
		for (int i = 0; i < currentPacketSize; i++) {
			cout << packetPayload[i];
		}
		cout << endl;*/

		uint16_t value = gen_crc16(packetPayload, currentPacketSize);
		cout << "checksum: " << value << endl;
		//uint16_t value2 = gen_crc16(packetPayload, sizeof(packetPayload));
		//cout << "Value2: " << value2 << endl;

		/*for (int i = 0; i < currentPacketSize; i++) {
			cout << *(packetPayload + i);
		}
		cout << endl;*/

		//packet vars
		int adjustedPayloadSize = 0;
		for (int i = 0; i < currentPacketSize; i++) {
			adjustedPayloadSize++;
			char currentPayloadChar = packetPayload[i];
			if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX || currentPayloadChar == DLE) {
				adjustedPayloadSize++;
			}
		}

		packet = new char[adjustedPayloadSize + PACKET_FRAME_SIZE];

		char packetNum = '0' + sequenceNumber;
		int packetIndex = 0;
		int packetPayloadIndex = 0;

		//asssign header of packet
		int zz = 0;
		packet[zz] = SOH;
		zz++;
		packet[zz] = packetNum;
		zz++;
		packet[zz] = STX;
		zz++;

		//assign payload of packet
		int i = PACKET_FRAME_SIZE - zz;
		while (packetPayloadIndex != currentPacketSize) {
			char currentPayloadChar = packetPayload[packetPayloadIndex];
			if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX || currentPayloadChar == DLE) {
				packet[i] = DLE;
				i++;
				packet[i] = currentPayloadChar;
				i++;
			}
			else {
				packet[i] = currentPayloadChar;
				i++;

			}
			packetPayloadIndex++;
		}

		//assign end of header
		packet[i] = ETX;

		short checksum = crcChecker.checksum();

		packet[i + 1] = (value >> 8);
		packet[i + 2] = (value & 0xFF);
		//cout << "Setting checsum to: " << (value >> 8) << " and " << (value & 0xFF) << endl;

		//print off values of packet
		cout << "Values in Packet: " << endl;
		for (int i = 0; i < (adjustedPayloadSize + PACKET_FRAME_SIZE); i++) {
			char character = *(packet + i);
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;

		//get updated time
		suseconds_t startWriteUSec = timer.GetTimeInMicroSeconds();

		bytes = write(socket, packet, adjustedPayloadSize + PACKET_FRAME_SIZE);
		if (bytes <= 0)
		{
			cout << "1. ERROR writing to socket: " << socket << endl;
			break;
		}
		else {
			totalBytes -= (bytes - PACKET_FRAME_SIZE);
			bytesWritten += bytes;
			numPacketsSent++;

			cout << "Packet " << sequenceNumber << " sent" << endl;
		}

		/**** READING ****/
		bytes = 0;
		bytes = read(socket, ackBuffer, sizeof(ack));
		if (bytes <= 0) {
			cout << "2. ERROR reading from socket: " << socket << endl;
			break;
		}
		else {
			//Get updated time
			suseconds_t endWriteUSec = timer.GetTimeInMicroSeconds();
			double rtt = endWriteUSec - startWriteUSec;

			cout << "Ack " << sequenceNumber << " received. (RTT for pkt " << sequenceNumber << " = " << rtt << "us)" << endl;
		}

		sequenceNumber++;
		sequenceNumber %= seqNumberRange;
	}

	// get updated time
	double Uduration = (timer.GetTimeInMicroSeconds() - startTimeUSecs);
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
int main(int argc, char **argv) {
	if (argc == 3) {
		int packetSizeInBytes = MAX_PACKET_SIZE;
		int seqNumberRange = MAX_SEQ_NUM_RANGE;
		char* ptr;
		// Max limit cannot be more than 2,147,483,647
		unsigned long portNum = strtol(argv[1], &ptr, 10); //returns 0 if argv[1] cannot be converted to an integer
		if (portNum <= 9000 || portNum > 10000) {
			cerr << "ERROR with second parameter: " << argv[1] << ", it should be a valid port number between 9,000 and 10,000" << endl;
		}
		else {
			//if this process should be the server
			if (!strcmp(argv[2], "-s")) {
				server(portNum);
			}
			else if (!strcmp(argv[2], "-c")) {
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

