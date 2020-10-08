#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define ADDRESS argv[1]
#define PORT argv[2]
#define PASSWORD argv[3]

/*Define offsets within packet*/
#define LENGTH *((int32_t *) packet)
#define ID *((int32_t *) (packet+4))
#define TYPE *((int32_t *) (packet+8))
#define PAYLOAD &packet[12]
#define FOOTER *((uint16_t *) (packet+LENGTH+2))

/*Define packet attributes*/
#define PACKET_MAX_SIZE 4100 /*4 bytes for length and 4096 for inner content*/
#define PACKET_MAX_CONTENT 4096
#define STRING_MAX_SIZE 4086 /*Maximum length for a string payload*/
#define PACKET_SIZE 10 /*Packets consist of two signed 32-bit integers and two ending bytes at minimum*/

int main(int argc, char *argv[]) {
	/*Argument check*/
	if(argc<4) {
		printf("Usage: %s ADDRESS PORT PASSWORD\n", argv[0]);
		exit(EXIT_FAILURE);
	}	

	/*Initial connection*/
	struct addrinfo *  _addrinfo;
	{
		int result;
		if((result=getaddrinfo(ADDRESS, PORT, NULL, &_addrinfo))) { 
			printf("%s\n", gai_strerror(result));
			freeaddrinfo(_addrinfo);
			exit(EXIT_FAILURE); 
		}
	}
	int _socket;
	if((_socket=socket((*_addrinfo).ai_family, SOCK_STREAM, 0))==-1) {
		perror("Failed to create socket");	
		freeaddrinfo(_addrinfo);
		exit(EXIT_FAILURE);
	}
	if(connect(_socket, (*_addrinfo).ai_addr, (*_addrinfo).ai_addrlen)==-1) {
		perror("Failed to connect socket");
		close(_socket); /*Just in case*/
		freeaddrinfo(_addrinfo);
		exit(EXIT_FAILURE);
	}
	printf("Connected\n");

	/*Authentication*/
	uint8_t packet[PACKET_MAX_SIZE];
	LENGTH=PACKET_SIZE+strlen(PASSWORD);
	ID=0;
	TYPE=3;
	memcpy(PAYLOAD, PASSWORD, strlen(PASSWORD)); 	
	FOOTER=0;

	write(_socket, packet, LENGTH+4);

	read(_socket, &LENGTH, 4);
	if(LENGTH>PACKET_MAX_CONTENT) {
		fprintf(stderr, "Invalid packet size received\n");
		close(_socket);
		freeaddrinfo(_addrinfo);
		exit(EXIT_FAILURE);
	}
	read(_socket, &packet[4], LENGTH);
	if(ID==-1) {
		fprintf(stderr, "Authentication failed\n");
		close(_socket);
		freeaddrinfo(_addrinfo);
		exit(EXIT_FAILURE);
	}
	printf("Authentication successful\n");


	/*Command loop*/
	char string[STRING_MAX_SIZE]; /*String buffer*/
	const struct timespec time={0, 10000000}; /*10 milliseconds*/
	printf("Waiting for input. Send EOF to exit.\n");
	while(1) {
		/*Get command*/
		putchar('>');
		if(fgets(string, STRING_MAX_SIZE, stdin)==NULL) {
			putchar('\n');
			break;
		}
		string[strlen(string)-1]=0; /*Remove final newline*/
		
		/*Send primary packet*/
		LENGTH=PACKET_SIZE+strlen(string);
		ID=0;
		TYPE=2;
		memcpy(PAYLOAD, string, strlen(string));
		FOOTER=0;
		write(_socket, packet, LENGTH+4);

		nanosleep(&time, NULL); /*Sleep before the next packet*/

		/*Send secondary packet (check for fragments)*/
		LENGTH=PACKET_SIZE;
		ID=1;
		TYPE=-1; 
		FOOTER=0;
		write(_socket, packet, LENGTH+4);

		/*Wait for packets to arrive*/
		while(1) {
			read(_socket, &LENGTH, 4);
			if(LENGTH>PACKET_MAX_CONTENT) {
				fprintf(stderr, "Invalid packet size received\n");
				close(_socket);
				freeaddrinfo(_addrinfo);
				exit(EXIT_FAILURE);
			}
			read(_socket, &packet[4], LENGTH);
			if(ID==1) /*Reached secondary packet*/
				break;
			printf("%s", PAYLOAD);
		}
		putchar('\n');
	}
	
	/*Clean up*/
	close(_socket);
	freeaddrinfo(_addrinfo);
	return 0;
}
