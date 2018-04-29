#include "server.h"
#include "client.h"
#include "userinput.h"
#include "pseudorandomgen.h"

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
				cout.precision(5);
				int packetSizeInBytes = DEFAULT_PACKET_SIZE;
				int seqNumberRange = DEFAULT_SEQ_NUM_RANGE;
				vector<int> acksToLose;
				vector<int> packetsToDamage;
				vector<int> packetsToDrop;

				string preset = GetPresets();

				if (preset == "SW") {
					client(portNum, 256, 2, "test.txt", Protocol::SW, -1, acksToLose, packetsToDamage, packetsToDrop, 1000);
				}
				else if (preset == "GBN") {
					cout << "Not configured";
					client(portNum, 256, 2, "test.txt", Protocol::GBN, 8, acksToLose, packetsToDamage, packetsToDrop, 500000);
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
						intervalTimeInMicroseconds = -1;
						string pingCommand = "ping thing3.cs.uwec.edu -s ";
						pingCommand += to_string(packetSizeInBytes);
						pingCommand += " -c 3";
						string commandOutput = executeCommand(pingCommand);

						//locate and extract the avg RRT given by the ping command's output
						size_t timesPos = commandOutput.find("= ");
						timesPos += 2;
						size_t endTimesPos = commandOutput.find(" ms", timesPos);
						string times = commandOutput.substr(timesPos, endTimesPos - timesPos);
						string chunk;
						vector<string> timesVector;
						stringstream stream(times);
						while (getline(stream, chunk, '/')) {
							timesVector.push_back(chunk);
						}

						size_t sz;

						//update the value retrieved to give a timeout
						double convertedAvgTimeout = stod(timesVector.at(1), &sz);
						intervalTimeInMicroseconds = (int)(convertedAvgTimeout * 15000); //convert it from ms to us
						cout << "Ping Calculated Timeout: " << intervalTimeInMicroseconds << "us" << endl;
						//cout << "Command Output: " << commandOutput << endl;
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
					else if(situationalErrorType == SitError::RG){
						int randErrorGenType = GetRandomErrorGenerationType();
						if (randErrorGenType == RandErrorGenType::A) {
							int numGeneratedErrors = GetNumberOfRandomlyGeneratedErrors(maxPackets);
							vector<long> randoms;
							setRandoms(seqNumberRange, numGeneratedErrors, randoms);

							//split up errors between the 3 types somewhat evenly
							int split = numGeneratedErrors / 3;
							int secondSplit = split * 2;
							for (int i = 0; i < numGeneratedErrors; i++) {
								if (i < split) {
									packetsToDrop.push_back(randoms.at(i));
								}
								else if (i <= secondSplit) {
									packetsToDamage.push_back(randoms.at(i));
								}
								else {
									acksToLose.push_back(randoms.at(i));
								}
							}
						}
						else {
							for (int i = 0; i < maxPackets; i++) {
								int randomChance = getARandom(100);
								//cout << "Chance to drop a packet: " << randomChance << endl;
								//random chance to add to packetsToDrop
								if (randomChance < getARandom(50)) {
									packetsToDrop.push_back(getARandom(seqNumberRange));
								}

								randomChance = getARandom(100);
								//cout << "Chance to damage a packet: " << randomChance << endl;
								//random chance to add to packetsToDamage
								if (randomChance < getARandom(50)) {
									packetsToDamage.push_back(getARandom(seqNumberRange));
								}

								randomChance = getARandom(100);
								//cout << "Chance to lose an ack: " << randomChance << endl;
								//random chance to add to acksToLose
								if (randomChance < getARandom(50)) {
									acksToLose.push_back(getARandom(seqNumberRange));
								}
							}
						}
					}

					cout << "Acks to lose: ";
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
					cout << endl;

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