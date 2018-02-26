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

using namespace std;
const char SOH = 0x7E;
const char EOP = 0x7E;
const char ACK = 0x06; //ACK Ascii character, also (0000 0110)
const int MAX_PACKET_SIZE = 2048; //payload size
const int MAX_SEQ_NUM_RANGE = 2; //number of available packet numbers starting with 0
const int CHAR_SIZE = sizeof(char);

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

string parseFile(int fileSize) {
	stringstream stream;
	char* chars = new char[fileSize];
	ifstream myfile("longtest.txt");
	int currentIndex = 0;
	if (myfile.good() && myfile.is_open()) {
		string input((istreambuf_iterator<char>(myfile)), istreambuf_iterator<char>());
		//cout << input << endl;
		return input;
	}
	else {
		cout << "ERROR: Could not open file" << endl;
		return "-1";
	}
}

int getFileSize() {
	string input;
	stringstream stream;
	ifstream myfile("longtest.txt", ifstream::binary);
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
	int ss = serverSocketSetup(portNum);
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);
	int packetSize = 0;
	int* packetSizePointer = &packetSize;
	int bytes = read(sockfd, packetSizePointer, sizeof(int));
	if (bytes <= 0) {
		cout << "1. ERROR reading from socket: " << sockfd << endl;
	}

	struct timeval time;
	gettimeofday(&time, NULL);

	double duration = 0.0;
	char* packetBuffer = new char[packetSize+3];
	time_t startTime = time.tv_sec;
	suseconds_t startTimeUSecs = time.tv_usec;
	//cout << "start time " << startTime << endl;
	while(1){
		/**** READING ****/
		cout << "Expected seq#: " << sequenceNumber << endl;
		bool foundEndOfPacket = false;
		int packetBufferIndex = 0;
		while (!foundEndOfPacket) {
			bytes = read(sockfd, packetBuffer + packetBufferIndex, 1);
			if (bytes <= 0) {
				//cout << "1. ERROR reading from socket: " << sockfd << endl;
				break;
			}
			else {
				char currentChar = *(packetBuffer + packetBufferIndex);
				if (packetBufferIndex != 0 && currentChar == EOP) {
					//end of packet so break loop
					foundEndOfPacket = true;
				}
				totalBytes += bytes;
				numPacketsReceived++;
				packetBufferIndex++;
				/*cout << "Server got value: " << endl;
				for (int i = 0; i < bytes; i++) {
				cout << *(packetBuffer + i) << "";
				}
				cout << endl;*/
			}
		}

		bytes = read(sockfd, packetBuffer, packetSize + 3);
		if(bytes <= 0){
			//cout << "1. ERROR reading from socket: " << sockfd << endl;
			break;
		}
		else {
			totalBytes += bytes;
			numPacketsReceived++;
			/*cout << "Server got value: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(packetBuffer + i) << "";
			}
			cout << endl;*/
		}
		
		/**** WRITING ****/
		bytes = 0;
		char packetNum = 'Z';

		for (int i = 0; i < packetSize; i++) {
			if (i == 1) {
				packetNum = *(packetBuffer + i);
			}
			else if (i > 1) {
				break;
			}
		}

		cout << "Packet " << packetNum << " received" << endl;
		sequenceNumber++;
		sequenceNumber %= 2;
		//send ack over to client with packet number
		char* ackMsg = &packetNum;
		
		bytes = write(sockfd, ackMsg, sizeof(packetNum));
		if(bytes <= 0){
			cout << "2. ERROR writing to socket: " << sockfd << endl;
			break;
		} 
		else {
			//cout << "Server wrote ack: " << *(ackMsg + 0) << endl;
			cout << "Ack " << *(ackMsg + 0) << " sent" << endl;
		}
		
		//cout << "To: thing2.cs.uwec.edu" << endl << endl;
	}

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

void client(int portNum, int packetSize, int seqNumberRange)
{
	int socket = callServer("thing3.cs.uwec.edu", portNum);
	int* intBuffer = &packetSize;
	int bytes = write(socket, intBuffer, sizeof(int));
	if (bytes <= 0)
	{
		cout << "1. ERROR writing to socket: " << socket << endl;
	}
	int payloadSize = packetSize - 3;
	int bytesWritten = 0;
	int totalBytes = 1;
	int numPacketsSent = 0;
	char ack;
	char* ackBuffer = &ack;
	int sequenceNumber = 0;
	double duration = 0.0;
	char* packetBuffer = new char[packetSize];
	for (int i = 0; i < packetSize; i++) {
		*(packetBuffer + i) = '0';
	}
	
	totalBytes = getFileSize();
	struct timeval time;
	gettimeofday(&time, NULL);
	time_t startTime = time.tv_sec;
	suseconds_t startTimeUSecs = time.tv_usec;
	int currentIndex = 0;
	string payload = parseFile(totalBytes);
	char packet[MAX_PACKET_SIZE];
	startTime = clock();

	while(totalBytes){
		int currentPacketSize = packetSize;
		if (totalBytes < packetSize) {
			currentPacketSize = totalBytes;
		}

		/**** WRITING ****/
		char* packetPayloadPointer = &payload[currentIndex];
		currentIndex += currentPacketSize;
		char* packetPayload = new char[currentPacketSize];
		strncpy(packetPayload, packetPayloadPointer, currentPacketSize);

		/*for (int i = 0; i < currentPacketSize; i++) {
			cout << *(packetPayload + i);
		}
		cout << endl;*/

		char packetNum = '0' + sequenceNumber;
		
		int packetIndex = 0;
		int packetPayloadIndex = 0;

		cout << "Current packet size " << currentPacketSize << endl;
		while(packetIndex != currentPacketSize + 3) {
			if (packetIndex == 0) {
				packet[packetIndex] = SOH;
			}
			else if (packetIndex == 1) {
				packet[packetIndex] = packetNum;
			}
			else if(packetIndex == currentPacketSize + 2){
				packet[packetIndex] = EOP;
			}
			else {
				packet[packetIndex] = packetPayload[packetPayloadIndex];
				packetPayloadIndex++;
			}
			packetIndex++;
		}

		packetBuffer = packet;

		/*cout << "Values in packet: " << endl;
		for (int i = 0; i < currentPacketSize + 3; i++) {
			cout << packet[i] << "";
		}
		cout << endl;*/
		gettimeofday(&time, NULL);
		suseconds_t startWriteUSec = time.tv_usec;
		bytes = write(socket, packetBuffer, currentPacketSize + 3);
		if(bytes <= 0)
		{
			cout << "1. ERROR writing to socket: " << socket << endl;
			break;
		}
		else {
			totalBytes -= bytes - 3;
			bytesWritten += bytes;
			numPacketsSent++;
			/*cout << "Client wrote value: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(packetBuffer + i) << "";
			}
			cout << endl;*/

			cout << "Packet " << sequenceNumber << " sent" << endl;
		}
		
		//cout << endl << "To: thing3.cs.uwec.edu" << endl << endl;
		
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
			gettimeofday(&time, NULL);
			suseconds_t endWriteUSec = time.tv_usec;
			double rtt = endWriteUSec - startWriteUSec;

			cout << "Ack " << sequenceNumber << " received. (RTT for pkt " << sequenceNumber << " = " << rtt << "us)" << endl;
		}

		sequenceNumber++;
		sequenceNumber %= seqNumberRange;
		
	}

	gettimeofday(&time, NULL);
	double Uduration = (time.tv_usec - startTimeUSecs);
	double throughput = ((double)bytesWritten / (double)Uduration);

	cout.precision(5);
	cout << endl << "Packet Size: " << bytesWritten << " bytes" << endl;
	cout << "Number of packets sent: " << numPacketsSent << endl;
	cout << "Total elapsed time: " << Uduration / 1000000 << "s" << endl;
	cout << "Throughput (Mbps): " << throughput << endl;
	cout << "md5sum: " << "0" << endl;

	close(socket);
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
				size_t* st;
				// GET Packet size from user
				bool validPacketSize = false;
				string tempPacketSize;
				while (!validPacketSize) {
					cout << "Please specify a packet size in bytes: " << endl;
					cin >> tempPacketSize;
					packetSizeInBytes = stoi(tempPacketSize, st, 10);
					if (packetSizeInBytes >= 1 && packetSizeInBytes <= MAX_PACKET_SIZE) {
						validPacketSize = true;
					}
					else {
						cout << "\nERROR: Packet size must be between 1 and " << MAX_PACKET_SIZE << " bytes, but you said: '" << tempPacketSize << "'" << endl;
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
				client(portNum, packetSizeInBytes, seqNumberRange);
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

