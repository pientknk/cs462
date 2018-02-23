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
	
}
 
void server(unsigned long limit, int portNum)
{
	int ss = serverSocketSetup(portNum);
	
	//listen to up to 2 connections
	listen(ss, 2);
	int sockfd = serverSocketAccept(ss);

	ssize_t bytes_to_write = 0;
	ssize_t bytes_received = 0;
	ssize_t bytes_to_receive;
	char a = 'A';
	char[] data = { a };
	char* buffer;
	
	while(1){
		/**** READING ****/
		bytes_received = 0;
		
		bytes = read(sockfd, buffer, sizeof(char));
		if(bytes <= 0){
			if(startingIndex >= largestIndex){
				break;
			}
			printf("1. ERROR reading from socket: %i\n", sockfd);
			break;
		} 
		
		/**** WRITING ****/
		bytes_written = 0;
		bytes = 0;
		
		//tell the reader what index to start at
		bytes = write(sockfd, buffer, sizeof(char));
		if(bytes <= 0){
			if(startingIndex >= largestIndex){
				break;
			}
			printf("2. ERROR writing to socket: %i\n", sockfd);
			break;
		} 
		
		//sent
		printf("To: thing2.cs.uwec.edu\n\n");
	}
	close(ss);
	close(sockfd);
}

void client(unsigned long limit, int portNum)
{
	int socket = callServer("thing3.cs.uwec.edu", portNum);

	ssize_t bytes_written = 0;
	ssize_t bytes_to_write;
	ssize_t bytes_received = 0;
	ssize_t bytes_to_receive;
	char a = 'A';
	char[] data = { a };
	char* buffer;
	//unsigned long *dataPointer;
	//dataPointer = (unsigned long *)malloc(UNSIGNED_LONG_SIZE * longLimit);

	while(1){
		/**** WRITING ****/
		bytes_written = 0;
		
		bytes = write(socket, buffer, sizeof(char));
		if(bytes <= 0)
		{
			if(startingIndex >= largestIndex){
				break;
			}
			printf("1. ERROR writing to socket: %i\n", socket);
			break;
		} 
		
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
