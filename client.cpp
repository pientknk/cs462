#include "client.h"
#include "printing.h"

int socket_;
int bytesWritten = 0;
int bytesResent = 0;
int numPacketsSent = 0;
unsigned long numAcksReceived = 0;
unsigned char ack;
unsigned char* ackBuffer = &ack;
int sequenceNumber_c = 0;
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
int adjustedPayloadSize_c = 0;

//Error introduction variables
bool sendWrongChecksums = false;
int j;

string executeCommand(string cmd) {
	string data;
	FILE * stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	cmd.append(" 2>&1");

	stream = popen(cmd.c_str(), "r");
	if (stream) {
		while (!feof(stream)) {
			if (fgets(buffer, max_buffer, stream) != NULL) {
				data.append(buffer);
			}
		}
			
		pclose(stream);
	}
	return data;
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

int writePacket(vector<int> &acksToLose, vector<int> &packetsToDamage, vector<int> &packetsToDrop, deque<unsigned char*> *windowContents, unsigned char* packetPointer) {
	unsigned char* packetPayload = new unsigned char[currentPacketSize];
	if(packetPointer == NULL){
		unsigned char* packetPayloadPointer = payload + currentIndex;
		memcpy(packetPayload, packetPayloadPointer, currentPacketSize);
	}
	else{
		memcpy(packetPayload, packetPointer, currentPacketSize);
	}

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
	adjustedPayloadSize_c = 0;
	for (int i = 0; i < currentPacketSize; i++) {
		adjustedPayloadSize_c++;
		unsigned char currentPayloadChar = packetPayload[i];
		if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX || currentPayloadChar == DLE) {
			adjustedPayloadSize_c++;
		}
	}

	packet_c = new unsigned char[adjustedPayloadSize_c + PACKET_FRAME_SIZE];
	
	//cout << "Size of packet: " << adjustedPayloadSize_c + PACKET_FRAME_SIZE << endl;

	unsigned char packetNum = '0' + sequenceNumber_c;
	int packetIndex = 0;
	int packetPayloadIndex = 0;

	//asssign header of packet
	int zz = 0;
	packet_c[zz] = SOH;
	zz++;

	/*cout << "ACks to lose before: " << endl;
	for (int i = 0; i < acksToLose.size(); i++) {
		cout << acksToLose.at(i) << " ";
	}
	cout << endl;*/

	//if we should drop the ack for this packet, then make seq num -1 so the server knows
	if (!acksToLose.empty()) {
		vector<int>::iterator it = find(acksToLose.begin(), acksToLose.end(), sequenceNumber_c);
		if (it != acksToLose.end()) {
			packet_c[zz] = 0xFF + '0';
			zz++;
			acksToLose.erase(it);
		}
		else {
			packet_c[zz] = packetNum;
			zz++;
		}
	}
	else {
		packet_c[zz] = packetNum;
		zz++;
	}
	
	/*cout << "ACks to lose after: " << endl;
	for (int i = 0; i < acksToLose.size(); i++) {
		cout << acksToLose.at(i) << " ";
	}
	cout << endl;*/
	
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

	/*cout << "packets to damage before: " << endl;
	for (int i = 0; i < packetsToDamage.size(); i++) {
		cout << packetsToDamage.at(i) << " ";
	}
	cout << endl;*/

	if (!packetsToDamage.empty()) {
		vector<int>::iterator it = find(packetsToDamage.begin(), packetsToDamage.end(), sequenceNumber_c);
		if (it != packetsToDamage.end()) {
			packet_c[i + 1] = (value >> 6);
			packet_c[i + 2] = (value & 0x7B);
			packetsToDamage.erase(it);
		}
		else {
			packet_c[i + 1] = (value >> 8);
			packet_c[i + 2] = (value & 0xFF);
		}
	}
	else {
		packet_c[i + 1] = (value >> 8);
		packet_c[i + 2] = (value & 0xFF);
	}
	
	if(windowContents != NULL){
		(*windowContents).push_back(packetPayload);
	}

	/*cout << "packets to damage after: " << endl;
	for (int i = 0; i < packetsToDamage.size(); i++) {
		cout << packetsToDamage.at(i) << " ";
	}
	cout << endl;*/

	//print off entire packet with * replacing
	//printPacketReplace(adjustedPayloadSize_c + PACKET_FRAME_SIZE, packet_c);

	//get updated time
	startWriteUSec = timer.GetCurrentTimeInMicroSeconds();
	
	return write(socket_, packet_c, adjustedPayloadSize_c + PACKET_FRAME_SIZE);
}

void client(int portNum, int packetSize, int seqNumberRange, string fileName, int protocol, int slidingWindowSize, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout) {
	socket_ = callServer("thing3.cs.uwec.edu", portNum);
	
	struct timeval sockTimeout;
	sockTimeout.tv_sec = timer.MicroSecToSec(intervalTimeout);
	sockTimeout.tv_usec = intervalTimeout;
	setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &sockTimeout, sizeof(struct timeval));
	
	//get file data and setup packet
	totalBytes_c = GetFileSize(fileName);
	payload = ParseFile(fileName, totalBytes_c);
	
	//Send over Packet Sequence Number Range information
	int* packetSizePointer = &seqNumberRange;
	if (!write(socket_, packetSizePointer, sizeof(int))) {
		cout << "1. ERROR reading from socket: " << socket_ << endl;
	}
	
	if (protocol == Protocol::SW) {
		char* protocolInfo = "SW";
		if (!write(socket_, protocolInfo, sizeof(protocolInfo))) {
			cout << "1. ERROR reading from socket: " << socket_ << endl;
		}
		clientStopAndWait(portNum, packetSize, seqNumberRange, fileName, acksToLose, packetsToDamage, packetsToDrop, intervalTimeout);
	}
	else{
		if (protocol == Protocol::GBN) {
			char* protocolInfo = "GBN";
			if (!write(socket_, protocolInfo, sizeof(protocolInfo))) {
			cout << "1. ERROR reading from socket: " << socket_ << endl;
		}
			clientGBN(portNum, packetSize, seqNumberRange, fileName, acksToLose, packetsToDamage, packetsToDrop, intervalTimeout, slidingWindowSize);
		}
		//selective repeat
		else {
	
		}
	}
	
	close(socket_);
}

void clientStopAndWait(int portNum, int packetSize, int seqNumberRange, string fileName, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout) {
	unsigned long maxPackets = ceil((double)totalBytes_c / (double)packetSize);

	//Time variables
	time_t startTimeUSecs = timer.GetCurrentTimeInMicroSeconds();

	while (totalBytes_c) {
		currentPacketSize = packetSize;
		if (totalBytes_c < packetSize) {
			currentPacketSize = totalBytes_c;
		}

		/**** WRITING ****/
		sendTime = timer.GetCurrentTimeInMicroSeconds();

		if (!packetsToDrop.empty()) {
			vector<int>::iterator it = find(packetsToDrop.begin(), packetsToDrop.end(), sequenceNumber_c);
			if (it != packetsToDrop.end()) {
				packetsToDrop.erase(it);
				cout << "**** Dropping packet " << sequenceNumber_c << " *****" << endl;

				int adjustedPayloadSize_c = 0;
				for (int i = 0; i < currentPacketSize; i++) {
					adjustedPayloadSize_c++;
					unsigned char currentPayloadChar = *(payload + currentIndex + i);
					if (currentPayloadChar == SOH || currentPayloadChar == STX || currentPayloadChar == ETX || currentPayloadChar == DLE) {
						adjustedPayloadSize_c++;
					}
				}

				extraBytes = adjustedPayloadSize_c + PACKET_FRAME_SIZE;
				bytes_c = currentPacketSize;
				bytesSent = bytes_c;
				numPacketsSent++;
			}
			else {
				extraBytes = writePacket(acksToLose, packetsToDamage, packetsToDrop, NULL, NULL);
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
			}
		}
		else {
			extraBytes = writePacket(acksToLose, packetsToDamage, packetsToDrop, NULL, NULL);
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
		}

		/**** READING ****/
		bytes_c = 0;
		bytes_c = read(socket_, ackBuffer, sizeof(ack));

		if (bytes_c <= 0) {
			//cout << "timer - sendtime: " << timer.GetCurrentTimeInMicroSeconds() - sendTime << " >= " << intervalTimeout << endl;
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
}

void clientGBN(int portNum, int packetSize, int seqNumberRange, string fileName, vector<int> acksToLose, vector<int> packetsToDamage, vector<int> packetsToDrop, int intervalTimeout, int sws) {
	int sendingWindowSize = sws;
	int lastAckReceived = -1;
	bool didReceiveLastAck = true;
	int lastFrameSent = -1;
	int expectedPacketNum = 0;
	unsigned long maxPackets = ceil((double)totalBytes_c / (double)packetSize);
	numPacketsSent = 0;
	int windowCapacity = 0;
	size_t* st;
	deque<unsigned char*> windowContents;
	bool noMorePackets = false;
	int tempTotalBytes = totalBytes_c;

	//Time variables
	time_t startTimeUSecs = timer.GetCurrentTimeInMicroSeconds();

	while (totalBytes_c >= 0) {
		currentPacketSize = packetSize;
		if (totalBytes_c < packetSize) {
			currentPacketSize = totalBytes_c;
		}
		
		/**** WRITING ****/
		sendTime = timer.GetCurrentTimeInMicroSeconds();
		
		while(lastFrameSent - lastAckReceived < sendingWindowSize) {
			if(!didReceiveLastAck){
				if(!windowContents.empty()){
					deque<unsigned char*>::iterator it = windowContents.begin();
					while(it != windowContents.end()){
						currentPacketSize = packetSize;
						if (tempTotalBytes < packetSize) {
							currentPacketSize = tempTotalBytes;
						}
		
						it++;
						extraBytes = writePacket(acksToLose, packetsToDamage, packetsToDrop, &windowContents, *it);
						tempTotalBytes -= currentPacketSize;
						bytes_c = currentPacketSize;

						if (extraBytes <= 0)
						{
							cout << "1. ERROR writing to socket: " << socket_ << endl;
							break;
						}
						else {
							bytesSent += adjustedPayloadSize_c + PACKET_FRAME_SIZE;;
							lastFrameSent = sequenceNumber_c;
							expectedPacketNum = lastFrameSent;
							windowCapacity++;
							numPacketsSent++;

							cout << "Packet " << sequenceNumber_c << " sent" << endl;
							sequenceNumber_c++;
						}
					}
				}
			}
			else{
				while(windowCapacity < sendingWindowSize && numPacketsSent < maxPackets){
					currentPacketSize = packetSize;
					if (tempTotalBytes < packetSize) {
						currentPacketSize = tempTotalBytes;
					}
		
					extraBytes = writePacket(acksToLose, packetsToDamage, packetsToDrop, &windowContents, NULL);
					tempTotalBytes -= currentPacketSize;
					currentIndex += currentPacketSize;
					
					bytes_c = currentPacketSize;

					if (extraBytes <= 0)
					{
						cout << "1. ERROR writing to socket: " << socket_ << endl;
						break;
					}
					else {
						bytesSent += adjustedPayloadSize_c + PACKET_FRAME_SIZE;
						lastFrameSent = sequenceNumber_c;
						expectedPacketNum = lastFrameSent;
						windowCapacity++;
						numPacketsSent++;

						cout << "Packet " << sequenceNumber_c << " sent" << endl;
						if(numPacketsSent < maxPackets) {
							sequenceNumber_c++;
						}
					}
				}
				
				if(numPacketsSent == maxPackets) {
					noMorePackets = true;
				}
			}
			
			if(noMorePackets) {
				break;
			}
		}

		/**** READING ****/
		bytes_c = 0;
		while(lastAckReceived != expectedPacketNum) {
			bytes_c = read(socket_, ackBuffer, sizeof(ack));

			if (bytes_c <= 0) {
				//cout << "timer - sendtime: " << timer.GetCurrentTimeInMicroSeconds() - sendTime << " >= " << intervalTimeout << endl;
				if (lastAckReceived < maxPackets) {
					cout << "Packet " << sequenceNumber_c << " **** Timed Out *****" << endl;
					cout << "Packet " << sequenceNumber_c << " Re-transmitted" << endl;
					bytesResent += extraBytes;
				}
				else {
					didReceiveLastAck = false;
				}
				
				break;
			}
			else {
				unsigned char ackChar = *(ackBuffer + 0);
				int ackValue = (ackChar - '0');
				
				if(ackValue >= lastAckReceived && ackValue <= lastFrameSent) {
					windowContents.pop_front();
					//Get updated time
					time_t readTime = timer.GetCurrentTimeInMicroSeconds();
					time_t rtt = readTime - sendTime;

					currentIndex += currentPacketSize;

					if (bytes_c > 0) {
						totalBytes_c -= bytesSent;
						extraBytes = bytesSent;
						bytesSent = 0;
					}

					cout << "Ack " << ackValue << " received. (RTT for pkt " << sequenceNumber_c << " = " << 	rtt << "us)" << endl;

					bytesWritten += extraBytes;
					
					sequenceNumber_c++;
					
					lastAckReceived = ackValue;
					numAcksReceived++;
					
					windowCapacity--;
				}
				else {
					cout << "Unexpected Ack: " << "Ack " << ackValue << " instead of Ack "<< expectedPacketNum << endl;
					didReceiveLastAck = false;
				}
			}
		}
	}

	if (totalBytes_c == 0) {
		cout << "Session successfully terminated" << endl;
	}
	else if (totalBytes_c < 0) {
		cout << "Session successfully terminated" << endl;
		//cout << "Session terminated with negative number of remaining bytes" << endl;
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
}