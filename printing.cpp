#include "printing.h"

void printClientStats(int bytesWritten, int bytesResent, int numPacketsSent, double throughput, double UDuration, const char* MD5Command) {
	//cout.precision(5);
	cout << endl << "Total bytes sent: " << bytesWritten + bytesResent << " bytes" << endl;
	cout << "Number of original packets sent: " << bytesWritten << " bytes" << endl;
	cout << "Number of retransmitted packets sent: " << bytesResent << " bytes" << endl;
	cout << "Number of packets sent: " << numPacketsSent << endl;
	cout << "Total elapsed time: " << UDuration / 1000000 << "s" << endl;
	cout << "Throughput (Mbps): " << throughput << endl;
	cout << "md5sum: " << endl;
	system(MD5Command);
	cout << endl;
}

void printServerStats(int lastPacketNum, int bytesReceived, int bytesDuplicate, int numPacketsReceived, double UDuration) {
	//cout.precision(5);
	cout << endl << "Last packet # received: " << lastPacketNum << endl;
	cout << "Total bytes received: " << bytesReceived + bytesDuplicate << " bytes" << endl;
	cout << "Number of original packets received: " << bytesReceived << " bytes" << endl;
	cout << "Number of retransmitted packets received: " << bytesDuplicate << " bytes" << endl;
	cout << "Number of packets received: " << numPacketsReceived << endl;
	cout << "Total elapsed time: " << UDuration / 1000000 << "s" << endl;
	cout << "md5sum: " << endl;
	system("md5sum Output.txt");
	cout << endl;
}

void printCheckSumIndividual(uint16_t value) {
	cout << "Calculated Checksum values: " << (value >> 8) << " and " << (value & 0xFF) << endl;
}

void printCheckSum(uint16_t value) {
	cout << "Calculated Checksum value: " << value << endl;
}

void printPacket(int packetBytes, unsigned char* packetBuffer) {
	cout << "Packet Size: " << packetBytes << " bytes" << endl;
	cout << "Packet:" << endl;
	for (int i = 0; i < packetBytes; i++) {
		cout << *(packetBuffer + i);
	}
	cout << endl;
}

void printPacketReplace(int packetBytes, unsigned char* packetBuffer) {
	cout << "Packet Size: " << packetBytes << " bytes" << endl;
	cout << "Packet:" << endl;
	for (int i = 0; i < packetBytes; i++) {
		char character = *(packetBuffer + i);
		if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
			if(character == SOH){
				cout << "(SOH)";
			}else if(character == STX){
				cout << "(STX)";
			}else if(character == ETX){
				cout << "(ETX)";
			}else if(character == DLE){
				cout << "(DLE)";
			}else{
				cout << "*";
			}
		}
		else {
			cout << character;
		}
	}
	cout << endl;
}

void printPayload(int packetSize, unsigned char* payload) {
	cout << "Packet Paylaod Size: " << packetSize << " bytes" << endl;
	cout << "Packet Payload:" << endl;
	for (int i = 0; i < packetSize; i++) {
		cout << *(payload + i);
	}
	cout << endl;
}

void printPayloadReplace(int packetSize, unsigned char* payload) {
	cout << "Packet Paylaod Size: " << packetSize << " bytes" << endl;
	cout << "Packet Payload (Replaced some special symbols with '*'):" << endl;
	for (int i = 0; i < packetSize; i++) {
		char character = *(payload + i);
		if (character == SOH || character == STX || character == ETX || character == DLE || character == NUL) {
			cout << "*";
		}
		else {
			cout << character;
		}
	}
	cout << endl;
}