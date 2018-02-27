TO SIMULATE PACKET STOP AND WAIT PROTOCOL:
	1. Place unzipped files into desired directory
	2. Open putty and log in to thing3 and thing2
	3. Navigate to directory of unzipped files on both thing2 and thing3
	4. On thing3, run make command
	5. On thing3, start the server with the following command
		./packet portNumber flag  
		ex) ./packet 9037 -s
	6. On thing2, start the client with the following command
		./packet portNumber flag  
		ex) ./packet 9037 -c

	7. If ERROR on Binding occurs: change portNumber
	8. make clean when done
		
