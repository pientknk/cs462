Packet::Packet(int pid, int burstTime, int arrival, int priority, int io, int deadline) {
	this->pid = pid;
	this->burstTime = burstTime;
	this->arrival = arrival;
	this->priority = priority;
	this->io = io;
	this->deadline = deadline;
	timeleft = burstTime;
	this->originalPriority = priority;
	this->originalIO = io;
}

void Process::setTimeLeft(int timeLeft) {
	timeleft = timeLeft;
}

void Process::setDeadline(int deadline) {
	this->deadline = deadline;
}

void Process::setWaitedTime(int wait) {
	waitedTime = wait;
}

void Process::setCurrentQuantum(int current) {
	currentQuantum = current;
}

void Process::setQueue(int queue) {
	currentQueue = queue;
}

void Process::setPriority(int newPriority) {
	priority = newPriority;
}

void Process::setIO(int newIO) {
	io = newIO;
}
