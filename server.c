#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>

//100 million at 16 min - 4 min 52 seconds
//10 million at 12.9 sec 
const int MAX_ARRAY_SIZE = 1000000;
const int UNSIGNED_LONG_SIZE = sizeof(unsigned long);
const int LONG_BITS = 32;

//initialize socket for the server
int serverSocketSetup(int portNum){
	//get the right socket file descriptor
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("ERROR opening this socket\n");
		exit(1);
	}
	
	// server address structure
	struct sockaddr_in serv_addr;
	
	//set all the valies in the server address to 0
	memset(&serv_addr, '0', sizeof(serv_addr));
	
	//setup the type of socket to be internet
	serv_addr.sin_family = AF_INET;
	
	//address of the machine we are on
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//setup the port number
	serv_addr.sin_port = htons(portNum);
	
	//bind the socket to the given port
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("ERROR with binding\n");
		exit(1);
	}
	
	return sockfd;
}

int callServer(char* host, int portNum){
	//socket pointer
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		fprintf(stderr, "ERROR opening socket\n");
		exit(0);
	}
	
	//server address structure
	struct sockaddr_in serv_addr;
	
	//set all the values in the server address to 0
	memset(&serv_addr, '0', sizeof(serv_addr));
	
	//setup the type of socket to be internet
	serv_addr.sin_family = AF_INET;
	
	//setup the port number
	serv_addr.sin_port = htons(portNum);
	
	//setup the server host address
	struct hostent *server;
	server = gethostbyname(host);
	if(server == NULL){
		fprintf(stderr, "Error, host '%s' was not found\n", host);
		exit(0);
	}
	
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length); //destination, source, sizeof
	
	//connect to the server
	if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("ERROR connecting to the server\n");
		exit(0);
	}
	
	return sockfd;
}

int serverSocketAccept(int serverSocket)
{
	int sockfd;
	struct sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	
	//wait for call
	printf("Waiting for a call...\n");
	sockfd = accept(serverSocket, (struct sockaddr *) &cli_addr, &clilen);
	printf("Connected\n");
	if(sockfd < 0){
		printf("ERROR accepting connection\n");
		exit(1);
	}
	
	return sockfd;
}

//flag 0 for receiving, 1 for sending
void printPrimes(int flag, int startingIndex, int longLimit, unsigned long *dataPointer, int limit){
	int printLimit = 3;
	int checkedNumber = 0;
	unsigned long lIndex = startingIndex / LONG_BITS;
	int sIndex = startingIndex % LONG_BITS;
	unsigned long numberToPrint = lIndex * LONG_BITS + 2 + sIndex;
	unsigned long firstLong = *(dataPointer + lIndex);
	unsigned long lastLong;
	int index = sIndex;
	if(flag == 0){
		printf("Recvd: ");
	} else{
		printf("Sent: ");
	}
	while(1){
		if(!((firstLong >> index) & 1)){
			checkedNumber++;
			printf("%i,", numberToPrint);
		} 
		if(checkedNumber == 3){
			checkedNumber = 0;
			break;
		}
		index++;
		numberToPrint++;
		if(index >= LONG_BITS){
			lIndex++;
			firstLong = *(dataPointer + lIndex);
			index = 0;
		}
	}
	//last 3 values
	int lastValues[3];
	int d = limit % LONG_BITS - 1;
	if(d <= 0){
		d = LONG_BITS - 1;
	}
	for(int z = (longLimit - 1); z >= 0; z--){
		if(checkedNumber >= 3){
			break;
		}
		unsigned long lastLong = *(dataPointer + z);
		for(d; d >= 0; d--){
			if(checkedNumber >= printLimit){
				break;
			}
			if(!(lastLong & (1 << d))){
				unsigned long intValue;
				if(z > 0){
					intValue = d + (z * 32) + 2;
				}
				else{
					intValue = d + 2;
				}
				
				if(intValue <= limit){
					lastValues[checkedNumber] = intValue;
					checkedNumber++;
				}
			} 
			if(d == 0){
				d = LONG_BITS - 1;
				break;
			}
		}
	}
	printf("...,%i,%i,%i\n", lastValues[2], lastValues[1], lastValues[0]);
}
 
void server(unsigned long limit, int portNum)
{
	int ss = serverSocketSetup(portNum);
	
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);

	/*** Loop Variables ***/
	unsigned long longLimit = limit / LONG_BITS;
	int longRemainder = limit % LONG_BITS;
	if(longRemainder != 0){
		longLimit++;
	}
	
	ssize_t bytes_written = 0;
	ssize_t bytes_to_write;
	ssize_t bytes_received = 0;
	ssize_t bytes_to_receive;
	ssize_t byte = 0;
	int longsReceived = 0;
	int longsWritten = 0;
	int bytes = 0;
	unsigned long startingIndex = 0; //the index of the number we are currently checking
	unsigned long *curIndexPointer;
	curIndexPointer = &startingIndex;
	int largestIndex = floor(sqrt(limit));
	unsigned long *dataPointer;
	dataPointer = (unsigned long *)malloc(UNSIGNED_LONG_SIZE * longLimit);
	int maxBytes = UNSIGNED_LONG_SIZE * longLimit;
	
	while(startingIndex <= largestIndex){
		/**** READING ****/
		longsReceived = 0;
		bytes_received = 0;
		
		//read in the starting index so we know where to start
		bytes = read(sockfd, curIndexPointer, UNSIGNED_LONG_SIZE);
		if(bytes <= 0){
			if(startingIndex >= largestIndex){
				break;
			}
			printf("1. ERROR reading from socket: %i\n", sockfd);
			break;
		} 
			
		while(bytes_received < maxBytes){
			bytes = read(sockfd, dataPointer + longsReceived, maxBytes);
			if(bytes <= 0){
				printf("ERROR reading from socket: %i\n", sockfd);
				break;
			} else{
				longsReceived += bytes / UNSIGNED_LONG_SIZE;
				bytes_received += bytes;
			}
		}
		
		/*** Print first and last 3 values ***/
		//received
		printPrimes(0, startingIndex, longLimit, dataPointer, limit);
		
		if(startingIndex > largestIndex){
			break;
		}
		
		/***** Eliminate Non Primes *****/
		int longIndex = startingIndex / LONG_BITS; //the long we should start with in the array
		int bitIndex = startingIndex % LONG_BITS; //the bit we should start with within the first long
		int currentNumber = startingIndex + 2; //the equivalent of current index but +2 for the number the bit represents - cuz we start at 2
		bitIndex += currentNumber; 
		while(1){ //jump to the needed long
			if(bitIndex >= LONG_BITS){
				longIndex++;
				bitIndex -= LONG_BITS;
			} else{
				break;
			}
		}
		bool done = false;
		while(1){
			while(1){ 
				*(dataPointer + longIndex) |= 1 << bitIndex;
				bitIndex += currentNumber;
				if(bitIndex >= LONG_BITS){
					break;
				}
			}
			
			if(bitIndex >= LONG_BITS){	
				while(1){
					if(bitIndex >= LONG_BITS){
						longIndex++;
						if(longIndex >= longLimit){
							done = true;
							break;
						}
						bitIndex -= LONG_BITS;
					} else{
						break;
					}
				}
			}
			
			if(done){
				break;
			}
		}
		
		/*** Get the next index that should be checked ***/
		startingIndex++;
		int currentLong = startingIndex / LONG_BITS;
		int currentBitIndex = startingIndex % LONG_BITS;
		while(1){
			//if bit is 1 we haven't found our next index to check
			if(*(dataPointer + currentLong) & (1 << currentBitIndex)){
				startingIndex++;
			} else{
				break;
			}
			currentBitIndex++;
			if(currentBitIndex >= LONG_BITS){
				currentBitIndex -= LONG_BITS;
				currentLong += 1;
			}
		}
		
		/**** WRITING ****/
		bytes_written = 0;
		bytes = 0;
		
		//tell the reader what index to start at
		bytes = write(sockfd, curIndexPointer, UNSIGNED_LONG_SIZE);
		if(bytes <= 0){
			if(startingIndex >= largestIndex){
				break;
			}
			printf("2. ERROR writing to socket: %i\n", sockfd);
			break;
		} 
		
		longsWritten = 0;
		while(bytes_written < maxBytes){
			bytes = write(sockfd, dataPointer + longsWritten, maxBytes);
			if(bytes <= 0){
				printf("ERROR writing to socket: %i\n", sockfd);
				break;
			} else{
				bytes_written += bytes;
				longsWritten += bytes / UNSIGNED_LONG_SIZE;
			}
		}
		
		/*** Print first and last 3 values ***/
		//sent
		printPrimes(1, startingIndex, longLimit, dataPointer, limit);
		printf("To: thing2.cs.uwec.edu\n\n");
	}
	free(dataPointer);
	close(ss);
	close(sockfd);
}

void client(unsigned long limit, int portNum)
{
	int socket = callServer("thing3.cs.uwec.edu", portNum);
	
	/*** Loop Variables ***/
	unsigned long longLimit = limit / LONG_BITS;
	int longRemainder = limit % LONG_BITS;
	if(longRemainder != 0)
	{
		longLimit++;
	}

	ssize_t bytes_written = 0;
	ssize_t bytes_to_write;
	ssize_t bytes_received = 0;
	ssize_t bytes_to_receive;
	ssize_t bytes = 0;
	int longsReceived = 0;
	int longsWritten = 0;
	unsigned long startingIndex = 0; //the index of the number we are currently checking
	unsigned long *curIndexPointer;
	curIndexPointer = &startingIndex;
	int largestIndex = floor(sqrt(limit));
	unsigned long *dataPointer;
	dataPointer = (unsigned long *)malloc(UNSIGNED_LONG_SIZE * longLimit);
	//initialize everything to 0 at first
	for(int dataIndex = 0; dataIndex < longLimit; dataIndex++){
		*(dataPointer + dataIndex) = 0;
	}
	int maxBytes = UNSIGNED_LONG_SIZE * longLimit;
	
	/***** Eliminate Non Primes *****/
	int longIndex = startingIndex / LONG_BITS; //the long we should start with in the array
	int bitIndex = startingIndex % LONG_BITS; //the bit we should start with within the first long
	int currentNumber = startingIndex + 2; //the equivalent of current index but +2 for the number the bit represents - cuz we start at 2
	
	//initial loop to get rid of EVENS
	while(1) {
		if(longIndex == 0){ //skip over first number in first long cuz it's prime, otherwise delete it
			bitIndex += currentNumber; 
		}
		while(1){ 
			*(dataPointer + longIndex) |= 1 << bitIndex;
			bitIndex += currentNumber;
			if(bitIndex >= LONG_BITS){
				break;
			}
		}
		
		if(bitIndex >= LONG_BITS){		
			longIndex++;
			if(longIndex >= longLimit){
				break;
			} else{
				bitIndex %= LONG_BITS; //go to the next bit based on the current increment
			}			
		}
	}
	
	startingIndex++; //we know this will be the next number initially (will be checking number 3)

	while(startingIndex <= largestIndex){
		/**** WRITING ****/
		bytes_written = 0;
		
		//tell the reader what index to start at
		bytes = write(socket, curIndexPointer, UNSIGNED_LONG_SIZE);
		if(bytes <= 0){
			if(startingIndex >= largestIndex){
				break;
			}
			printf("1. ERROR writing to socket: %i\n", socket);
			break;
		} 
		
		longsWritten = 0;
		while(bytes_written < maxBytes){
			bytes = write(socket, dataPointer + longsWritten, maxBytes);
			if(bytes <= 0)
			{
				printf("ERROR writing to socket: %i\n", socket);
				break;
			} else {
				bytes_written += bytes;
				longsWritten += bytes / UNSIGNED_LONG_SIZE;
			}
		}
		
		/*** Print first and last 3 values ***/
		//sent
		printPrimes(1, startingIndex, longLimit, dataPointer, limit);
		printf("To: thing3.cs.uwec.edu\n\n");
		
		/**** READING ****/
		longsReceived = 0;
		bytes_received = 0;
		bytes = 0;
		
		//read in the starting index so we know where to start
		bytes = read(socket, curIndexPointer, UNSIGNED_LONG_SIZE);
		if(bytes <= 0){
			if(startingIndex >= largestIndex){
				break;
			}
			printf("2. ERROR reading from socket: %i\n", socket);
			break;
		} 
		
		longsReceived = 0;
		while(bytes_received < maxBytes){
			bytes = read(socket, dataPointer + longsReceived, maxBytes);
			if(bytes <= 0)
			{
				printf("ERROR reading from socket: %i\n", socket);
				break;
			} else{
				longsReceived += bytes / UNSIGNED_LONG_SIZE;
				bytes_received += bytes;
			}
		}
		
		/*** Print first and last 3 values ***/
		//received
		printPrimes(0, startingIndex, longLimit, dataPointer, limit);
		
		if(startingIndex > largestIndex){
			break;
		}
		
		/***** Eliminate Non Primes *****/
		longIndex = startingIndex / LONG_BITS; //the long we should start with in the array
		bitIndex = startingIndex % LONG_BITS; //the bit we should start with within the first long
		currentNumber = startingIndex + 2; //the equivalent of current index but +2 for the number the bit represents - cuz we start at 2
		bitIndex += currentNumber; 
		while(1){ //jump to the needed long
			if(bitIndex >= LONG_BITS){
				longIndex++;
				bitIndex -= LONG_BITS;
			} else{
				break;
			}
		}
		bool done = false;
		while(1){	
			while(1){ 
				*(dataPointer + longIndex) |= 1 << bitIndex;
				bitIndex += currentNumber;
				if(bitIndex >= LONG_BITS){
					break;
				}
			}
			
			if(bitIndex >= LONG_BITS){	
				while(1){
					if(bitIndex >= LONG_BITS){
						longIndex++;
						if(longIndex >= longLimit){
							done = true;
							break;
						}
						bitIndex -= LONG_BITS;
					} else{
						break;
					}
				}
			}
			
			if(done){
				break;
			}
		}
		
		/*** Get the next index that should be checked ***/
		startingIndex++;
		int currentLong = startingIndex / LONG_BITS;
		int currentBitIndex = startingIndex % LONG_BITS;
		while(1){
			//if bit is 1 we haven't found our next index to check
			if(*(dataPointer + currentLong) & (1 << currentBitIndex)){
				startingIndex++;
			} else{
				break;
			}
			currentBitIndex++;
			if(currentBitIndex >= LONG_BITS){
				currentBitIndex -= LONG_BITS;
				currentLong += 1;
			}
		}
		
		if(startingIndex >= largestIndex){
			bytes_written = 0;
		
			//tell the reader what index to start at
			bytes = write(socket, curIndexPointer, UNSIGNED_LONG_SIZE);
			if(bytes <= 0){
				if(startingIndex >= largestIndex){
					break;
				}
				printf("ERROR writing to socket: %i\n", socket);
				break;
			} 
			
			longsWritten = 0;
			while(bytes_written < maxBytes){
				bytes = write(socket, dataPointer + longsWritten, maxBytes);
				if(bytes <= 0){
					printf("ERROR writing to socket: %i\n", socket);
					break;
				} else{
					bytes_written += bytes;
					longsWritten += bytes / UNSIGNED_LONG_SIZE;
				}
			}
			//sending
			printPrimes(1, startingIndex, longLimit, dataPointer, limit);
			printf("To: thing3.cs.uwec.edu\n\n");
			break;
		}
	}
	free(dataPointer);
	close(socket);
}

//should pass in filename, maxLimit, flag
//TO RUN SERVER: ./primes 1000 9036 -s
//TO RUN CLIENT: ./primes 1000 9036 -c
int main(int argc, char **argv){
	if(argc == 4){
		char* ptr;
		// Max limit cannot be more than 2,147,483,647
		unsigned long maxLimit = strtol(argv[1], &ptr, 10); //returns 0 if argv[1] cannot be converted to an integer
		if(maxLimit <= 0){
			printf("ERROR with second parameter: %s, it should be a positive valid unsigned long\n", argv[1]);
		} else{
			char* portPtr;
			int portNumber = strtol(argv[2], &portPtr, 10);
			if(portNumber <= 9000 || portNumber > 10000){
				printf("ERROR with third parameter: %s, it should be between 9,000 and 10,000\n", argv[2]);
			} else{
				//if this process should be the server
				if(!strcmp(argv[3], "-s")){
					server(maxLimit, portNumber);
				} else if(!strcmp(argv[3], "-c")){
					client(maxLimit, portNumber);
				} else{
					printf("ERROR with third parameter: %s, the proper flags are -c or -s\n", argv[2]);
				}
			}
		}
	}
	else{
		printf("Incorrect number of arguments. Should be something like ./getprimes 1000 s\n");
	}
}
