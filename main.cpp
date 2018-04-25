#include "server.h"
#include "client.h"
#include "userinput.h"

//TO RUN SERVER: ./packet 9036 -s
//TO RUN CLIENT: ./packet 9036 -c
int main(int argc, char **argv) {
	if (argc == 3) {
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
				int packetSizeInBytes = DEFAULT_PACKET_SIZE;
				int seqNumberRange = DEFAULT_SEQ_NUM_RANGE;
				vector<int> acksToLose;
				vector<int> packetsToDamage;
				vector<int> packetsToDrop;

				string preset = GetPresets();

				if (preset == "SW") {
					clientStopAndWait(portNum, 256, 2, "test.txt", 1200);
				}
				else if (preset == "GBN") {
					cout << "Not configured";
					//client(portNum, packetSizeInBytes, seqNumberRange, fileName, protocol, slidingWindowSize, acksToLose, packetsToDamage, packetsToDrop, intervalTimeInMicroseconds);
				}
				else if (preset == "SR") {
					cout << "Not configured";
					//client(portNum, packetSizeInBytes, seqNumberRange, fileName, protocol, slidingWindowSize, acksToLose, packetsToDamage, packetsToDrop, intervalTimeInMicroseconds);
				}
				else {
					string fileName = GetFileName();
					packetSizeInBytes = GetPacketSize();

					unsigned long maxPackets = ceil((double)GetFileSize(fileName) / (double)packetSizeInBytes);

					int protocol = GetProtocol();
					int slidingWindowSize = 0;
					if (protocol == Protocol::GBN || protocol == Protocol::SR) {
						slidingWindowSize = GetSlidingWindowSize(maxPackets);
					}

					int intervalMethod = GetTimeOutIntervalMethod();
					int intervalTimeInMicroseconds = -1;
					if (intervalMethod == TOInterval::US) {
						intervalTimeInMicroseconds = GetTimeOutFromUser();
					}
					else { //ping calculated
						   //use ping to pick a good timeout? or pass in -1 so that the client know that it needs to figure it out
					}

					seqNumberRange = GetSequenceNumberRange(maxPackets);
					int situationalErrorType = GetSituationalErrorType();

					if (situationalErrorType == SitError::USP) {
						int errorControlType = GetErrorControlType();
						//ack lost
						if (errorControlType == ErrorControl::AL) {
							int numAcksToLose = GetNumberOfAcksToLose(maxPackets);
							acksToLose = GetAllAcksToLose(numAcksToLose);
						}
						//packet damage
						else if (errorControlType == ErrorControl::PD) {
							int numPacketsToDamage = GetNumberOfPacketsToDropOrDamage(maxPackets, false);
							packetsToDamage = GetAllPacketsToDropOrDamage(numPacketsToDamage, false);
						}
						//packet loss/drop
						else if (errorControlType == ErrorControl::PL) {
							int numPacketsToDamage = GetNumberOfPacketsToDropOrDamage(maxPackets, true);
							packetsToDrop = GetAllPacketsToDropOrDamage(numPacketsToDamage, true);
						}
						//multiple
						else {
							GetMultipleErrorsFromUser(maxPackets, packetsToDrop, packetsToDamage, acksToLose);
						}
					}

					/*cout << "Acks to lose: ";
					for(int value : acksToLose)
					{
					cout << value << ", ";
					}
					cout << endl;

					cout << "Packets to damage: ";
					for(int value : packetsToDamage)
					{
					cout << value << ", ";
					}
					cout << endl;

					cout << "Packets to drop: ";
					for(int value : packetsToDrop)
					{
					cout << value << ", ";
					}
					cout << endl;*/

					client(portNum, packetSizeInBytes, seqNumberRange, fileName, protocol, slidingWindowSize, acksToLose, packetsToDamage, packetsToDrop, intervalTimeInMicroseconds);
				}
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