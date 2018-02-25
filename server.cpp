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

void printStats(long packetSize, int numPacketsSent, int numPacketsReceived, double totalTime, double throughput, string md5Sum) {
	cout << endl << "Packet Size: " << packetSize << "bytes" << endl;
	if (numPacketsSent != 0) {
		cout << "Number of packets sent: " << numPacketsSent << endl;
	}
	else {
		cout << "Number of packets received: " << numPacketsReceived << endl;
	}

	cout << endl << "Total elapsed time: " << totalTime << endl;
	cout << "Throughput (Mbps): " << throughput << endl;
	cout << "md5sum: " << md5Sum << endl;
}
 
void server(int portNum, int packetSize)
{
	int ss = serverSocketSetup(portNum);
	
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);

	ssize_t bytes_to_write = 0;
	ssize_t bytes_received = 0;
	ssize_t bytes_to_receive;
	ssize_t bytes_written;
	int bytes = 0;
	char* packetBuffer = new char[packetSize];
	for (int i = 0; i < packetSize; i++) {
		packetBuffer[i] = '0';
	}
	
	while(1){
		/**** READING ****/
		bytes_received = 0;
		
		bytes = read(sockfd, packetBuffer, sizeof(packetSize * CHAR_SIZE));
		if(bytes <= 0){
			cout << "1. ERROR reading from socket: " << sockfd << endl;
			break;
		}
		else {
			cout << "Server got value: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(packetBuffer + i) << "";
			}
			cout << endl;
		}
		
		/**** WRITING ****/
		bytes_written = 0;
		bytes = 0;
		char packetNum = 'Z';

		for (int i = 0; i < packetSize; i++) {
			if (i == 1) {
				packetNum = *(packetBuffer + i);
			}
		}

		//send ack over to client with packet number
		char packet = packetNum;
		char* ackMsg = &packet;
		
		bytes = write(sockfd, ackMsg, sizeof(packet));
		if(bytes <= 0){
			cout << "2. ERROR writing to socket: " << sockfd << endl;
			break;
		} 
		else {
			cout << "Server wrote ack: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(ackMsg + i);
			}
			cout << endl;
		}
		
		cout << "To: thing2.cs.uwec.edu" << endl << endl;

		break;
	}
	close(ss);
	close(sockfd);
}

void client(int portNum, int packetSize, int seqNumberRange)
{
	int socket = callServer("thing3.cs.uwec.edu", portNum);

	ssize_t bytes_written = 0;
	ssize_t bytes_to_write;
	ssize_t bytes_received = 0;
	ssize_t bytes_to_receive;
	int bytes = 0;
	char ack[]{ 'Z', 'Z' };
	char* ackBuffer = ack;
	int sequenceNumber = 0;
	char a = 'A';

	char* packetBuffer = new char[packetSize];
	for (int i = 0; i < packetSize; i++) {
		*(packetBuffer + i) = '0';
	}

	char charsWritten = 0;

	while(1){
		/**** WRITING ****/
		bytes_written = 0;
		char* str;
		char packetNum = '0' + sequenceNumber;
		char packet[] = { SOH, packetNum, a, EOP};
		packetBuffer = packet;
		charsWritten = 0;

		cout << "Values in packet: " << endl;
		for (int i = 0; i < sizeof(packet); i++) {
			cout << packet[i] << "";
		}
		cout << endl;

		bytes = write(socket, packetBuffer, sizeof(packetSize * CHAR_SIZE));
		if(bytes <= 0)
		{
			cout << "1. ERROR writing to socket: " << socket << endl;
			break;
		}
		else {
			cout << "Client wrote value: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(packetBuffer + i) << "";
			}
		}
		
		cout << endl << "To: thing3.cs.uwec.edu" << endl << endl;
		
		/**** READING ****/
		bytes_received = 0;
		bytes = 0;
		
		bytes = read(socket, ackBuffer, sizeof(ack));
		if(bytes <= 0){
			cout << "2. ERROR reading from socket: " << socket << endl;
			break;
		}
		else {
			cout << "Client got ACK: " << endl;
			for (int i = 0; i < bytes; i++) {
				cout << *(ackBuffer + i) << "";
			}
			cout << endl;
		}
	}
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
				server(portNum, packetSizeInBytes);
			} else if(!strcmp(argv[2], "-c")){
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

