#include "client.h"
#include "printing.h"

int socket_;
int bytesWritten = 0;
int bytesResent = 0;
int numPacketsSent = 0;
unsigned long numAcksReceived = 0;
unsigned char ack;
unsigned char* ackBuffer = &ack;
int sequenceNumber_c;
int bytes_c;
long totalBytes_c;
int bytesSent;
int currentIndex = 0;
unsigned char* packet_c;
unsigned char* payload;
int currentPacketSize;
Timer timer;
time_t startWriteUSec;
time_t sendTime;
int extraBytes = 0;

//Error introduction variables
bool sendWrongChecksums = false;
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

int writePacket(vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop) {
	unsigned char* packetPayloadPointer = payload + currentIndex;
	unsigned char* packetPayload = new unsigned char[currentPacketSize];
	memcpy(packetPayload, packetPayloadPointer, currentPacketSize);

	//print off current packet payload from file
	//printPayload(currentPacketSize, packetPayload);

	//print off packet payload after copying it over from the file, replacing some chars
	//printPayloadReplace(currentPacketSize, packetPayload);

	/*string contents = "";

	for (int i = 0; i < currentPacketSize; i++) {
		unsigned char character = *(packetPayload + i);
		contents += character;
	}

	ofstream afile;
	afile.open("ClientOutput.txt");
	afile << contents;
	afile.close();
	cout << "Wrote file ClientOutput.txt" << endl;*/

	uint16_t value = gen_crc16(packetPayload, currentPacketSize);
	//printCheckSumIndividual(value);
	//printCheckSum(value);

	//packet vars
	int adjustedPayloadSize = 0;
	for (int i = 0; i < currentPacketSize; i++) {
		adjustedPayloadSize++;
		unsigned char currentPayloadChar = packetPayload[i];
		if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX || currentPayloadChar == DLE) {
			adjustedPayloadSize++;
		}
	}

	packet_c = new unsigned char[adjustedPayloadSize + PACKET_FRAME_SIZE];
	
	//cout << "Size of packet: " << adjustedPayloadSize + PACKET_FRAME_SIZE << endl;

	unsigned char packetNum = '0' + sequenceNumber_c;
	int packetIndex = 0;
	int packetPayloadIndex = 0;

	//asssign header of packet
	int zz = 0;
	packet_c[zz] = SOH;
	zz++;
	//if we should drop the ack for this packet, then make seq num 255 so the server knows
	vector<int>::iterator it = find(acksToLose.begin(), acksToLose.end(), sequenceNumber_c);
	if (it != acksToLose.end()) {
		packet_c[zz] = 255;
		zz++;
		//TODO: remove this from the vector now?
	}
	else {
		packet_c[zz] = packetNum;
		zz++;
	}
	
	packet_c[zz] = STX;
	zz++;

	//assign payload of packet
	int i = PACKET_FRAME_SIZE - zz;
	while (packetPayloadIndex != currentPacketSize) {
		unsigned char currentPayloadChar = packetPayload[packetPayloadIndex];
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

	if (sendWrongChecksums) {
		if (j == 2) {
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

	//print off entire packet with * replacing
	//printPacketReplace(adjustedPayloadSize + PACKET_FRAME_SIZE, packet_c);

	//get updated time
	startWriteUSec = timer.GetCurrentTimeInMicroSeconds();
	
	return write(socket_, packet_c, adjustedPayloadSize + PACKET_FRAME_SIZE);
}

void client(int portNum, int packetSize, int seqNumberRange, string fileName, int protocol, int slidingWindowSize, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout)
{
	if (protocol == Protocol::SW) {
		clientStopAndWait(portNum, packetSize, seqNumberRange, fileName, acksToLose, packetsToDamage, packetsToDrop, intervalTimeout);
	}
	else if (protocol == Protocol::GBN) {
		//clientGBN(portNum, packetSize, seq);
	}
	//selective repeat
	else {

	}
}

//send over 255 (max user allowed is 254) as seq num so the server know's that it should "lose the ack"
void clientStopAndWait(int portNum, int packetSize, int seqNumberRange, string fileName, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout) {
	socket_ = callServer("thing3.cs.uwec.edu", portNum);

	struct timeval sockTimeout;
	sockTimeout.tv_sec = 0;
	sockTimeout.tv_usec = intervalTimeout;
	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &sockTimeout, sizeof(struct timeval));

	//get file data and setup packet
	totalBytes_c = GetFileSize(fileName);
	payload = ParseFile(fileName, totalBytes_c);

	unsigned long maxPackets = ceil((double)totalBytes_c / (double)packetSize);

	//Time variables
	time_t startTime = timer.GetTimeInSeconds();
	time_t startTimeUSecs = timer.GetCurrentTimeInMicroSeconds();
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
		sendTime = timer.GetCurrentTimeInMicroSeconds();
		extraBytes = writePacket(acksToLose, packetsToDamage, packetsToDrop);
		bytes_c = currentPacketSize;

		if (extraBytes <= 0)
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
			cout << "timer - sendtime: " << timer.GetCurrentTimeInMicroSeconds() - sendTime << " >= " << intervalTimeout << endl;
			if (numAcksReceived < maxPackets) {
				cout << "Packet " << sequenceNumber_c << " **** Timed Out *****" << endl;
				cout << "Packet " << sequenceNumber_c << " Re-transmitted" << endl;
				bytesResent += extraBytes;
			}
			else {
				break;
			}
		}
		else {
			//Get updated time
			time_t readTime = timer.GetCurrentTimeInMicroSeconds();
			time_t rtt = readTime - sendTime;

			currentIndex += currentPacketSize;

			if (bytes_c > 0) {
				totalBytes_c -= bytesSent;
			}

			cout << "Ack " << sequenceNumber_c << " received. (RTT for pkt " << sequenceNumber_c << " = " << rtt << "us)" << endl;

			bytesSent = extraBytes;
			bytesWritten += bytesSent;
			sequenceNumber_c++;
			sequenceNumber_c %= seqNumberRange;
			numAcksReceived++;

		}
	}

	if (totalBytes_c == 0) {
		cout << "Session successfully terminated" << endl;
	}
	else if (totalBytes_c < 0) {
		cout << "Session terminated with negative number of remaining bytes" << endl;
	}
	else {
		cout << "Session terminated early" << endl;
	}

	// get updated time
	double Uduration = (timer.GetCurrentTimeInMicroSeconds() - startTimeUSecs);
	double throughput = ((double)bytesWritten / (double)Uduration);

	const char* systemMD5 = ("md5sum " + fileName).c_str();

	//print stats
	printClientStats(bytesWritten, bytesResent, numPacketsSent, throughput, Uduration, systemMD5);

	close(socket_);
}