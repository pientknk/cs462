
//if we want this as a class rather than a struct
Packet::Packet(uint8_t sof) {
	this->sof = sof;
}

void Packet::getSof() {
	this->sof = sof;
}
