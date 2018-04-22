CC = g++
CFLAGS =  -std=c++11 -lm -pedantic

CCFILES = main.cpp server.cpp client.cpp packet.cpp files.cpp crc16Checksum.cpp timer.cpp

OFILES = main.o server.o client.o packet.o files.o crc16Checksum.o timer.o

all: link

link: $(OFILES)
	$(CC) -g $(CFLAGS) -o packet $(OFILES)
	
main.o: main.cpp server.cpp client.cpp
	$(CC) -c main.cpp $(CFLAGS)
	
server.o: server.cpp server.h
	$(CC) -c server.cpp $(CFLAGS)
	
client.o: client.cpp client.h
	$(CC) -c client.cpp $(CFLAGS)
	
crc16Checksum.o: crc16Checksum.cpp crc16Checksum.h
	$(CC) -c crc16Checksum.cpp $(CFLAGS)
	
files.o: files.cpp files.h
	$(CC) -c files.cpp $(CFLAGS)
	
clean:
	rm -f *.o *.*~ packet

