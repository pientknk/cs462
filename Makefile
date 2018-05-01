CC = g++
CFLAGS =  -std=c++11 -lm -pedantic

CCFILES = main.cpp server.cpp client.cpp filehandler.cpp crc16Checksum.cpp timer.cpp userinput.cpp printing.cpp pseudorandomgen.cpp

OFILES = main.o server.o client.o filehandler.o crc16Checksum.o timer.o userinput.o printing.o pseudorandomgen.o

all: link

link: $(OFILES)
	$(CC) -g $(CFLAGS) -o packet $(OFILES)
	
main.o: main.cpp server.cpp client.cpp constants.h pseudorandomgen.cpp
	$(CC) -c main.cpp $(CFLAGS)
	
server.o: server.cpp server.h
	$(CC) -c server.cpp $(CFLAGS)
	
client.o: client.cpp client.h
	$(CC) -c client.cpp $(CFLAGS)
	
crc16Checksum.o: crc16Checksum.cpp crc16Checksum.h
	$(CC) -c crc16Checksum.cpp $(CFLAGS)
	
filehandler.o: filehandler.cpp filehandler.h
	$(CC) -c filehandler.cpp $(CFLAGS)
	
userinput.o: userinput.cpp userinput.h
	$(CC) -c userinput.cpp $(CFLAGS)
	
printing.o: printing.cpp printing.h
	$(CC) -c printing.cpp $(CFLAGS)
	
pseudorandomgen.o: pseudorandomgen.cpp pseudorandomgen.h
	$(CC) -c pseudorandomgen.cpp $(CFLAGS)
	
clean:
	rm -f *.o *.*~ packet

