#include "userinput.h"

//global variables
size_t* st;
char* charPtr;
locale loc;
string input;
bool hasValidInput;
const string NANERROR = "\nERROR: You must enter a number and it has to be greater than 0";

string GetPresets() {
	hasValidInput = false;
	input = "";
	while (!hasValidInput) {
		cout << "Would you like to use the presets? Type the name of the preset to use: SW, GBN, or SR. Or type 'n' to continue" << endl;
		cin >> input;
		ifstream myfile(input);
		if (input == "SW" || input == "GBN" || input == "SR") {
			hasValidInput = true;
		}
		else if (input == "n") {
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: invalid input provided: " << input << endl;
		}
	}

	return input;
}

string GetFileName() {
	hasValidInput = false;
	input = "";
	while (!hasValidInput) {
		cout << "Please give the name of the file to send: " << endl;
		cin >> input;
		ifstream myfile(input);
		if (myfile.good() && myfile.is_open()) {
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: unable to open file: " << input << endl;
		}
	}

	return input;
}

int GetPacketSize() {
	int packetSizeInBytes = 0;
	hasValidInput = false;
	input = "";
	while (!hasValidInput) {
		cout << "Please specify a packet size in bytes: " << endl;
		cin >> input;
		packetSizeInBytes = strtol(input.c_str(), &charPtr, 10);
		if (packetSizeInBytes) {
			if (packetSizeInBytes >= 1 && packetSizeInBytes <= MAX_PACKET_SIZE) {
				hasValidInput = true;
			}
			else {
				cout << "\nERROR: Packet size must be between 1 and " << MAX_PACKET_SIZE << " bytes" << endl;
			}
		}
		else {
			cout << NANERROR << endl;
		}
	}

	return packetSizeInBytes;
}

int GetSequenceNumberRange(int maxPackets) {
	hasValidInput = false;
	int seqNumberRange = 0;
	input = "";
	while (!hasValidInput) {
		cout << "Please specify the range of sequence numbers: (Current number of Packets: " << maxPackets << ")" << endl;
		cin >> input;
		seqNumberRange = strtol(input.c_str(), &charPtr, 10);
		if (seqNumberRange) {
			if (seqNumberRange >= 1 && seqNumberRange <= MAX_SEQ_NUM_RANGE) {
				hasValidInput = true;
			}
			else {
				cout << "\nERROR: Sequence number range must be within 1-" << MAX_SEQ_NUM_RANGE << ", but you said: '" << seqNumberRange << "'" << endl;
			}
		}
		else {
			cout << NANERROR << endl;
		}
	}

	return seqNumberRange;
}

int GetProtocol() {
	hasValidInput = false;
	input = "";
	int protocol = Protocol::SW;
	while (!hasValidInput) {
		cout << "Please select a protocol. 0 for SW (Stop and Wait), 1 for GBN, or 2 for SR" << endl;
		cin >> input;
		if (input == Protocols[Protocol::SW] || input == Protocols[Protocol::GBN] || input == Protocols[Protocol::SR]) {
			protocol = stoi(input, st, 10);
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must select protocol 0, 1, or 2" << endl;
		}
	}

	return protocol;
}

int GetTimeOutIntervalMethod() {
	hasValidInput = false;
	input = "";
	int theInterval = TOInterval::PC;
	while (!hasValidInput) {
		cout << "Please select a Time Out Interval Method. 0 for User-Specified, or 1 for Ping-Calculated" << endl;
		cin >> input;
		if (input == TOIntervals[TOInterval::PC] || input == TOIntervals[TOInterval::US]) {
			theInterval = stoi(input, st, 10);
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must select Time Out Interval Method 0, or 1" << endl;
		}
	}

	return theInterval;
}

int GetTimeOutFromUser() {
	hasValidInput = false;
	input = "";
	int theInterval = 1;
	while (!hasValidInput) {
		cout << "How long should the timeout be in microseconds?" << endl;
		cin >> input;
		theInterval = strtol(input.c_str(), &charPtr, 10);
		if (theInterval) {
			if (theInterval >= MIN_TIMEOUT && theInterval <= MAX_TIMEOUT) {
				hasValidInput = true;
			}
			else {
				cout << "\nERROR: You must enter a number that is between " << MIN_TIMEOUT << " and " << MAX_TIMEOUT <<  " microseconds (" << (double)MAX_TIMEOUT / (double)1000000 << " sec)" << endl;
			}
		}
		else {
			cout << NANERROR << endl;
		}
	}

	return theInterval;
}

int GetSlidingWindowSize(int maxPackets) {
	hasValidInput = false;
	input = "";
	int size = 1;
	while (!hasValidInput) {
		cout << "What should the Sliding Window Size be? (Current number of Packets: " << maxPackets << ")" << endl;
		cin >> input;
		size = strtol(input.c_str(), &charPtr, 10);
		if (size > 0 && size <= 16) {
			if (size != 1 && size % 2 != 0) {
				cout << "\nERROR: You must enter a number that is between 1 and 16 and a multiple of 2" << endl;
			}
			else {
				hasValidInput = true;
			}
		}
		else {
			cout << "\nERROR: You must enter a number that is between 1 and 16 and a multiple of 2" << endl;
		}
	}

	return size;
}

int GetSituationalErrorType() {
	hasValidInput = false;
	input = "";
	int errorType = SitError::NO;
	while (!hasValidInput) {
		cout << "Please select a Situational Error Type. 0 for User-Specified, 1 for Randomly Generated, or 2 for None" << endl;
		cin >> input;
		if (input == SitErrors[SitError::NO] || input == SitErrors[SitError::RG] || input == SitErrors[SitError::USP]) {
			errorType = stoi(input, st, 10);
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must select a Situational Error Type 0, 1, or 2" << endl;
		}
	}

	return errorType;
}

int GetRandomErrorGenerationType() {
	hasValidInput = false;
	input = "";
	int randGenErrorType = RandErrorGenType::P;
	while (!hasValidInput) {
		cout << "Please select a Random Error Generation Type. 0 for Percent, or 1 for Amount." << endl;
		cin >> input;
		if (input == RandErrorGenTypes[RandErrorGenType::P] || input == RandErrorGenTypes[RandErrorGenType::A]) {
			randGenErrorType = stoi(input, st, 10);
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must select a Random Error Generation Type 0, or 1" << endl;
		}
	}

	return randGenErrorType;
}

int GetNumberOfRandomlyGeneratedErrors(int maxNumberOfPackets) {
	hasValidInput = false;
	input = "";
	int numGeneratedErrors = 0;
	while (!hasValidInput) {
		cout << "Please enter the number of randomly generated errors (must be less than or equal to " << maxNumberOfPackets << ")" << endl;
		cin >> input;
		numGeneratedErrors = strtol(input.c_str(), &charPtr, 10);
		if (numGeneratedErrors > 0 || numGeneratedErrors <= maxNumberOfPackets) {
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must enter a valid number that is between 1-" << maxNumberOfPackets << endl;
		}
	}

	return numGeneratedErrors;
}

int GetErrorControlType() {
	hasValidInput = false;
	input = "";
	int errorType = ErrorControl::AL;
	while (!hasValidInput) {
		cout << "Please select an Error Control Type. 0 for Packet Loss, 1 for Packet Damage, 2 for Ack Loss, or 3 for multiple" << endl;
		cin >> input;
		if (input == ErrorControls[ErrorControl::AL] || input == ErrorControls[ErrorControl::ML] || input == ErrorControls[ErrorControl::PD] || input == ErrorControls[ErrorControl::PL]) {
			errorType = stoi(input, st, 10);
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must select an Error Control Type 0, 1, 2, or 3" << endl;
		}
	}

	return errorType;
}

int GetNumberOfPacketsToDropOrDamage(int maxNumberOfPackets, bool isDropping) {
	hasValidInput = false;
	input = "";
	int numberOfPackets = 0;
	while (!hasValidInput) {
		if (isDropping) {
			cout << "How many packets do you want to DROP? (Current number of Packets: " << maxNumberOfPackets << ")" << endl;
		}
		else {
			cout << "How many packets do you want to DAMAGE? (Current number of Packets: " << maxNumberOfPackets << ")" << endl;
		}
		
		cin >> input;
		numberOfPackets = strtol(input.c_str(), &charPtr, 10);
		if (numberOfPackets) {
			if (numberOfPackets > 0 && numberOfPackets <= maxNumberOfPackets) {
				hasValidInput = true;
			}
			else {
				cout << "\nERROR: You must enter a number that is between 1 and " << maxNumberOfPackets << endl;
			}
		}
		else {
			cout << NANERROR << endl;
		}
	}

	return numberOfPackets;
}

vector<int> GetAllPacketsToDropOrDamage(int numPacketsToDrop, bool isDropping) {
	hasValidInput = false;
	input = "";
	vector<int> packetsToDrop;
	while (!hasValidInput) {
		if (isDropping) {
			cout << "Please give a list of the packets you would like to DROP. For example: 1,4,9,15 (Current number of Packets: " << numPacketsToDrop << ")" << endl;
		}
		else {
			cout << "Please give a list of the packets you would like to DAMAGE. For example: 1,4,9,15 (Current number of Packets: " << numPacketsToDrop << ")" << endl;
		}
		
		cin >> input;
		stringstream ss(input);
		while (ss.good()) {
			string substr;
			getline(ss, substr, ',');
			int value = strtol(substr.c_str(), &charPtr, 10);
			if (value) {
				packetsToDrop.push_back(value);
			}
			else {
				cout << NANERROR << endl;
				packetsToDrop.clear();
				break;
			}
		}

		if (packetsToDrop.size() == numPacketsToDrop) {
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You need to enter " << numPacketsToDrop << " packet numbers, you entered " << packetsToDrop.size() << " numbers" << endl;
		}
	}

	return packetsToDrop;
}

int GetNumberOfAcksToLose(int maxNumberOfPackets) {
	hasValidInput = false;
	input = "";
	int numberOfAcks = 0;
	while (!hasValidInput) {
		cout << "How many ACKS do you want to LOSE? (Current number of Packets: " << maxNumberOfPackets << ")" << endl;
		cin >> input;
		numberOfAcks = strtol(input.c_str(), &charPtr, 10);
		if (numberOfAcks > 0 && numberOfAcks <= maxNumberOfPackets) {
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You must enter a number that is between 1 and " << maxNumberOfPackets << endl;
		}
	}

	return numberOfAcks;
}

vector<int> GetAllAcksToLose(int numAcksLost) {
	hasValidInput = false;
	input = "";
	vector<int> acksToDrop;
	while (!hasValidInput) {
		cout << "Please give a list of the ACKS you would like to LOSE. For example: 1,4,9,15 (Current number of Packets: " << numAcksLost << ")" << endl;
		cin >> input;
		stringstream ss(input);
		while (ss.good()) {
			string substr;
			getline(ss, substr, ',');
			int value = strtol(substr.c_str(), &charPtr, 10);
			if (value) {
				acksToDrop.push_back(value);
			}
			else {
				cout << NANERROR << endl;
				acksToDrop.clear();
				break;
			}
		}

		if (acksToDrop.size() == numAcksLost) {
			hasValidInput = true;
		}
		else {
			cout << "\nERROR: You need to enter " << numAcksLost << " ack numbers, you entered " << acksToDrop.size() << " numbers" << endl;
		}
	}

	return acksToDrop;
}

void GetMultipleErrorsFromUser(int maxNumberOfPackets, vector<int>& droppedPackets, vector<int>& damagedPackets, vector<int>& acksLost) {
	//get droppped packets vector
	int numPacketsToDrop = GetNumberOfPacketsToDropOrDamage(maxNumberOfPackets, true);
	droppedPackets = GetAllPacketsToDropOrDamage(numPacketsToDrop, true);

	//get damaged packets vector
	int numPacketsToDamage = GetNumberOfPacketsToDropOrDamage(maxNumberOfPackets, false);
	damagedPackets = GetAllPacketsToDropOrDamage(numPacketsToDamage, false);

	//get acks to lose
	int numAcksToLose = GetNumberOfAcksToLose(maxNumberOfPackets);
	acksLost = GetAllAcksToLose(numAcksToLose);
}

//TO RUN SERVER: ./packet 9036 -s
//TO RUN CLIENT: ./packet 9036 -c
//int main(int argc, char **argv) {
//	if (argc == 3) {
//		int packetSizeInBytes = MAX_PACKET_SIZE;
//		int seqNumberRange = MAX_SEQ_NUM_RANGE;
//		char* ptr;
//		unsigned long portNum = strtol(argv[1], &ptr, 10);
//		if (portNum <= 9000 || portNum > 10000) {
//			cerr << "ERROR with second parameter: " << argv[1] << ", it should be a valid port number between 9,000 and 10,000" << endl;
//		}
//		else {
//			//if this process should be the server
//			if (!strcmp(argv[2], "-s")) {
//				server(portNum);
//			}
//			else if (!strcmp(argv[2], "-c")) {
//				string fileName = GetFileName();
//
//				int protocol = GetProtocol();
//				int slidingWindowSize = 0;
//				if (protocol == Protocol::GBN || protocol == Protocol::SR) {
//					slidingWindowSize = GetSlidingWindowSize()
//				}
//
//				packetSizeInBytes = GetPacketSize();
//
//				int intervalMethod = GetTimeOutIntervalMethod();
//				int intervalTimeInMicroseconds = -1;
//				if (intervalMethod == TOInterval::US) {
//					intervalTimeInMicroseconds = GetTimeOutFromUser();
//				}
//				else {
//					//use ping to pick a good timeout? or pass in -1 so that the client know that it needs to figure it out
//				}
//
//				seqNumberRange = GetSequenceNumberRange();
//
//				int situationalErrorType = GetSituationalErrorType();
//				if (situationalErrorType == SitErrors[SitError::US]) {
//					int errorControlType = GetErrorControlType();
//					//ack lost
//					if (errorControlType == ErrorControls[ErrorControl::AL]) {
//
//					}
//					//packet damage
//					else if (errorControlType == ErrorControls[ErrorControl::PD]) {
//
//					}
//					//packet loss
//					else if (errorControlType == ErrorControls[ErrorControl::PL]) {
//
//					}
//					//multiple
//					else {
//
//					}
//				}
//
//				client(portNum, packetSizeInBytes, seqNumberRange, fileName);
//			}
//			else {
//				cerr << "ERROR with third parameter: " << argv[2] << ", the proper flags are -c or -s" << endl;
//			}
//		}
//	}
//	else {
//		cerr << "ERROR: Incorrect number of arguments. Command should be something like ./packet portNum -flag (s or c)" << endl;
//	}
//
//	exit(0);
//}