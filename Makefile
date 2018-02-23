GPP = gcc
CFLAGS = -Wall

all: server

server: server.o Makefile
	$(GPP) server.o -o server

server.o: server.cpp
	$(GPP) -c server.cpp $(CFLAGS)

clean:
	rm -f *.o server
