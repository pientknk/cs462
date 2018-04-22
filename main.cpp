#include "server.h"
#include "client.h"
#include "userinput.h"

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
					cout << endl;
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