#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "process.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits.h>
#include <algorithm>
#include <queue>
#include <math.h>
#include <deque>
#include <map>

using namespace std;

typedef struct Packet {
	uint8_t sof;
	uint32_t src;
	uint32_t dst;
	uint8_t seqn;

};

int main() {
	// GET SCHEDULER
	bool choseScheduler = false;
	string scheduler;
	while (!choseScheduler) {
		cout << "Please specify a process scheduler: rts, mfqs, or whs" << endl;
		cin >> scheduler;
		if (scheduler == "rts" || scheduler == "mfqs" || scheduler == "whs") {
			choseScheduler = true;
		}
		else {
			cout << "\nERROR: Incorrect scheduler '" << scheduler << "' specified, expected rts, mfqs, or whs" << endl;
		}
	}

	// GET INPUT FROM FILE OR USER
	bool choseFileOrInput = false;
	string inputType;
	while (!choseFileOrInput) {
		cout << "Do you want to run " << scheduler << " with a file(f) or manual(m) input for processes?" << endl;
		cin >> inputType;
		if (inputType == "f" || inputType == "m") {
			choseFileOrInput = true;
		}
		else {
			cout << "\nERROR: Incorrect input type '" << inputType << "' specified, expected f or m" << endl;
		}
	}

	// GET PROCESSES TO RUN
	vector<Process> processes;
	stringstream stream;
	string input;
	array<int, 6> in;
	for (int i = 0; i < (int)in.size(); i++) {
		in[i] = -1;
	}
	if (inputType == "f") {
		bool choseInputFile = false;
		while (!choseInputFile) {
			cout << "Enter File Name: " << endl;
			string fileName;
			cin >> fileName;
			ifstream myfile(fileName);
			if (myfile.good() && myfile.is_open()) {
				choseInputFile = true;
				while (getline(myfile, input)) {
					stream.str(input);
					stream >> in[0] >> in[1] >> in[2] >> in[3] >> in[4] >> in[5];
					stream.clear();
					Process newProcess = Process(in[0], in[1], in[2], in[3], in[5], in[4]);
					if (scheduler == "mfqs") {
						if (newProcess.getArrival() >= 0 && newProcess.getPriority() >= 0 && newProcess.getBurstTime() > 0 && newProcess.getPid() != 0) {
							processes.push_back(newProcess);
						}
					}
					else if (scheduler == "rts") {
						if (newProcess.getArrival() >= 0 && newProcess.getBurstTime() > 0 && newProcess.getArrival() < newProcess.getDeadline() && newProcess.getDeadline() > 0) {
							processes.push_back(newProcess);
						}
					}
					else if(scheduler == "whs"){
						if (newProcess.getArrival() >= 0 && newProcess.getPriority() >= 0 && newProcess.getBurstTime() > 0 && newProcess.getPid() != 0 && newProcess.getIO() >= 0) {
							processes.push_back(newProcess);
						}
					}
					else {
						cout << "ERROR: Unknown scheduler passed in" << endl;
					}
				}

				sort(processes.begin(), processes.end(), [](const Process& left, const Process& right)
				{
					return left.getArrival() < right.getArrival();
				});
			}
			else {
				cout << "\nERROR: Could not find valid file '" << fileName << "'" << endl;
			}
		}
	}
	else if (inputType == "m") {
		bool hasAllInput = false;
		vector<string> inputParts;
		cout << "Enter Process Information: ProcessID   Burst   Arrival     Priority    Deadline    I/O" << endl;
		cout << "Example Process: 1 8 0 23 10 3" << endl;
		cout << "Hit Enter to input another process or hit Enter then q then Enter to finish" << endl;
		while (!hasAllInput) {
			string process;
			getline(cin, process);
			if (process == "q") {
				hasAllInput = true;
			}
			else if(process != ""){
				int count = 0;
				std::istringstream iss(process);
				for (string s; iss >> s; ) {
					count++;
				}
				if (count == 6) {
					inputParts.push_back(process);
					//cout << "Added process: " << process << endl;
					cout << "Added process. Processes added: " << inputParts.size() << endl;
				}
				else {
					cout << "ERROR: Expected 6 values but got " << count << endl;
				}
			}
		}

		for (auto & inputPart : inputParts) {
			stream.str(inputPart);
			stream >> in[0] >> in[1] >> in[2] >> in[3] >> in[4] >> in[5];
			stream.clear();
			Process newProcess = Process(in[0], in[1], in[2], in[3], in[5], in[4]);
			cout << "Process(" << in[0] << ", " << in[1] << ", " << in[2] << ", " << in[3] << ", " << in[5] << ", " << in[4] << ")" << endl;
			if (newProcess.getArrival() >= 0 && newProcess.getPriority() >= 0 && newProcess.getBurstTime() > 0) {
				processes.push_back(newProcess);
			}
		} 

		sort(processes.begin(), processes.end(), [](const Process& left, const Process& right)
		{
			return left.getArrival() < right.getArrival();
		});
	}
	else {
		cout << "\nERROR: Unknown input type specified" << endl;
	}

	//RUN SCHEDULERS
	if (scheduler == "rts") {
		bool choseRTSVersion = false;
		string rtsVersion;
		while (!choseRTSVersion) {
			cout << "Hard Real-Time(h) or Soft Real-Time?(s)" << endl;
			cin >> rtsVersion;
			if (rtsVersion == "h" || rtsVersion == "s") {
				choseRTSVersion = true;
			}
			else {
				cout << "\nERROR: Uknown Real-Time Scheduler '" << rtsVersion << "' specified. Must be h or s" << endl;
			}
		}

		RTS(processes, rtsVersion);
	}
	else if (scheduler == "mfqs") {
		int numQueues;
		int ageNum;
		int quantumBurst;
		bool choseQueueCount = false;
		while (!choseQueueCount) {
			cout << "Enter number of queues:" << endl;
			cin >> numQueues;
			if (numQueues > 0 && numQueues < 6) {
				choseQueueCount = true;
			}
			else {
				cout << "\nERROR: Number of queues must be between 1 and 5" << endl;
			}
		}

		bool choseQuantumBurst = false;
		while (!choseQuantumBurst) {
			cout << "Enter Quantum Burst time:" << endl;
			cin >> quantumBurst;
			if (quantumBurst > 0) {
				choseQuantumBurst = true;
			}
			else {
				cout << "\nERROR: Quantum burst time must be greater than 0" << endl;
			}
		}

		bool choseAgingNumber = false;
		while (!choseAgingNumber) {
			cout << "Enter Aging time:" << endl;
			cin >> ageNum;
			if (ageNum > 0) {
				choseAgingNumber = true;
			}
			else {
				cout << "\nERROR: Aging time must be greater than 0" << endl;
			}
		}

		MFQS(processes, numQueues, quantumBurst, ageNum);
	}
	else if (scheduler == "whs") {
		int quantumBurst;
		bool choseQuantum = false;
		while (!choseQuantum) {
			cout << "Enter Quantum Burst time:" << endl;
			cin >> quantumBurst;
			if (quantumBurst > 0) {
				choseQuantum = true;
			}
			else {
				cout << "\nERROR: Quantum Burst must be greater than 0" << endl;
			}
		}
		WHS(processes, quantumBurst);
	}
	else {
		cout << "\nERROR: Unknown Scheduler requested";
	}

	exit(0);
}

void printStats(double completed, double waitingTime, double turnaroundTime) {
	if (completed != 0) {
		cout << endl << "Average Waiting Time (AWT): " << (double)waitingTime / completed << endl;
		cout << "Average Turnaround Time (ATT): " << (double)turnaroundTime / completed << endl;
		cout << "Total number of processes scheduled (NP): " << completed << endl; //??? should this be any process that had CPU time? not just completed?
	}
	else {
		cout << endl << "Average Waiting Time (AWT): 0" << endl;
		cout << "Average Turnaround Time (ATT): 0" << endl;
		cout << "Total number of processes scheduled (NP): 0" << endl;
	}
}

void frame() {
	//Start of frame (SOF) (8 bits)
	//SRC IPv4 (32 bits)
	//DST IPV4 (32 bits)
	//SEQ# (8 bits)
	//Payload (100 bytes)

}
