CC = g++
CFLAGS =  -std=c++11 -g -lm -pedantic

CCFILES = server.cpp

OFILES = $(CCFILES:.cpp=.cpp)

all: link

link: $(OFILES)
	$(CC) $(CFLAGS) -o packet $(OFILES)
clean:
	rm -f *.o *.*~ packet
