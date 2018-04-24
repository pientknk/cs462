#include "client.h"

int socket_;
int bytesWritten = 0;
int bytesResent = 0;
int numPacketsSent = 0;
char ack;
char* ackBuffer = &ack;
int sequenceNumber_c;
int bytes_c;
long totalBytes_c;
int bytesSent;
int currentIndex = 0;
char* packet_c;
char* payload;
int currentPacketSize;
Timer timer;
suseconds_t startWriteUSec;
time_t sendTime;

//Error introduction variables
bool sendWrongChecksums = true;
int j;

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

int writePacket() {
		char* packetPayloadPointer = payload + currentIndex;
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

		uint16_t value = gen_crc16(packetPayload, currentPacketSize);
		//cout << "checksum: " << value << endl;
		
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

		packet_c = new char[adjustedPayloadSize + PACKET_FRAME_SIZE];

		char packetNum = '0' + sequenceNumber_c;
		int packetIndex = 0;
		int packetPayloadIndex = 0;

		//asssign header of packet
		int zz = 0;
		packet_c[zz] = SOH;
		zz++;
		packet_c[zz] = packetNum;
		zz++;
		packet_c[zz] = STX;
		zz++;

		//assign payload of packet
		int i = PACKET_FRAME_SIZE - zz;
		while (packetPayloadIndex != currentPacketSize) {
			char currentPayloadChar = packetPayload[packetPayloadIndex];
			if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX || currentPayloadChar == DLE) {
				packet_c[i] = DLE;
				i++;
				packet_c[i] = currentPayloadChar;
				i++;
			}
			else {
				packet_c[i] = currentPayloadChar;
				i++;

			}
			packetPayloadIndex++;
		}

		//assign end of header
		packet_c[i] = ETX;

		if(sendWrongChecksums) {
			if(j == 2) {
				packet_c[i + 1] = (value >> 8);
				packet_c[i + 2] = (value & 0xFF);
				
				j = 0;
			}
			else {
				packet_c[i + 1] = (value >> 6);
				packet_c[i + 2] = (value & 0x7B);
				
				j++;
			}
		}
		else {
			packet_c[i + 1] = (value >> 8);
			packet_c[i + 2] = (value & 0xFF);
		}
		//cout << "Setting checsum to: " << (value >> 8) << " and " << (value & 0xFF) << endl;

		//print off values of packet
		/*cout << "Values in packet_c: " << endl;
		for (int i = 0; i < (adjustedPayloadSize + PACKET_FRAME_SIZE); i++) {
			char character = *(packet_c + i);
			if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
				cout << "*";
			}
			else {
				cout << character;
			}
		}
		cout << endl;*/

		//get updated time
		startWriteUSec = timer.GetTimeInMicroSeconds();

		return write(socket_, packet_c, adjustedPayloadSize + PACKET_FRAME_SIZE);
}

void client(int portNum, int packetSize, int seqNumberRange, string fileName, int protocol, int slidingWindowSize, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop)
{
	socket_ = callServer("thing3.cs.uwec.edu", portNum);
	
	struct timeval sockTimeout;
	sockTimeout.tv_sec = 1;
	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &sockTimeout, sizeof(struct timeval));

	//get file data and setup packet
	totalBytes_c = GetFileSize(fileName);
	payload = ParseFile(fileName, totalBytes_c);

	//print out file contents
	/*cout << "Value of File: " << endl;
	for (int i = 0; i < totalBytes_c; i++) {
		char character = *(payload + i);
		if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
			cout << "*";
		}
		else {
			cout << character;
		}
	}
	cout << endl;*/

	//Time variables
	time_t startTime = timer.GetTimeInSeconds();
	suseconds_t startTimeUSecs = timer.GetTimeInMicroSeconds();
	double duration = 0.0;

	//Send over Packet Sequence Number Range information
	int* packetSizePointer = &seqNumberRange;
	if (!write(socket_, packetSizePointer, sizeof(int))) {
		cout << "1. ERROR reading from socket: " << socket_ << endl;
	}

	while (totalBytes_c) {
		currentPacketSize = packetSize;
		if (totalBytes_c < packetSize) {
			currentPacketSize = totalBytes_c;
		}

		/**** WRITING ****/
		bytes_c = writePacket();
		sendTime = timer.GetTimeInSeconds();
		
		if (bytes_c <= 0)
		{
			cout << "1. ERROR writing to socket: " << socket_ << endl;
			break;
		}
		else {
			bytesSent = bytes_c;
			numPacketsSent++;

			cout << "Packet " << sequenceNumber_c << " sent" << endl;
		}
		
		/**** READING ****/
		bytes_c = 0;
		bytes_c = read(socket_, ackBuffer, sizeof(ack));

		if (bytes_c <= 0) {	
			//cout << "2. ERROR reading from socket: " << socket_ << endl;
			if(timer.GetTimeInSeconds() - sendTime >= 1) {
				cout << "Packet " << sequenceNumber_c << " **** Timed Out *****" << endl;
				cout << "Packet " << sequenceNumber_c << " Re-transmitted" << endl;
				bytesResent += bytesSent;
			}
			else {
				break;
			}
		}
		else {
			currentIndex += currentPacketSize;
			
			//Get updated time
			suseconds_t endWriteUSec = timer.GetTimeInMicroSeconds();
			double rtt = endWriteUSec - startWriteUSec;

			if(bytes_c > 0) {
				totalBytes_c -= (bytesSent - PACKET_FRAME_SIZE);
			}
			
			cout << "Ack " << sequenceNumber_c << " received. (RTT for pkt " << sequenceNumber_c << " = " << rtt << "us)" << endl;
			
			bytesWritten += bytesSent;
			sequenceNumber_c++;
			sequenceNumber_c %= seqNumberRange;
		}
	}
	
	if(totalBytes_c == 0) {
		cout << "Session successfully terminated" << endl;
	}
	else if (totalBytes_c < 0) {
		cout << "Session terminated with negative number of remaining bytes" << endl;
	}
	else {
		cout << "Session terminated early" << endl;
	}

	// get updated time
	double Uduration = (timer.GetTimeInMicroSeconds() - startTimeUSecs);
	double throughput = ((double)bytesWritten / (double)Uduration);
	
	const char* systemMD5 = ("md5sum " + fileName).c_str();

	//print stats
	cout.precision(5);
	cout << endl << "Total bytes sent: " << bytesWritten + bytesResent << " bytes" << endl;
	cout << "Number of original packets sent: " << bytesWritten << " bytes" << endl;
	cout << "Number of retransmitted packets sent: " << bytesResent << " bytes" << endl; 
	cout << "Number of packets sent: " << numPacketsSent << endl;
	cout << "Total elapsed time: " << Uduration / 1000000 << "s" << endl;
	cout << "Throughput (Mbps): " << throughput << endl;
	cout << "md5sum: " << endl;
	system(systemMD5);
	cout << endl;

	close(socket_);

	//cout << endl << "To: thing3.cs.uwec.edu" << endl << endl;
}