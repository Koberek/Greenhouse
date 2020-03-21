// Server side implementation of UDP client-server model 

//	https://www.geeksforgeeks.org/udp-server-client-implementation-c/


#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define PORT	8080 
#define MAXLINE 1024 

int len, n;

// Driver code 
int main() { 
	printf("UDP Server Running... \n");		//Only text printed to screen
	
	int sockfd; 
	char buffer[MAXLINE]; 
	//char *hello = "Hello from server"; 
	
	// Assembled NTP packet for testing. 
	uint8_t myPacket[48] = {0x1C,0x01,0x0D,0xE3,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x20,
							0x4E,0x49,0x53,0x54,0xE2,0x20,0xA8,0xE4,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE2,0x20,0xA9,0x40,
							0xCE,0x24,0xFD,0x69,0xE2,0x20,0xA9,0x40,0xCE,0x25,0x11,0x83};
	
	
	struct sockaddr_in servaddr, cliaddr; 
	
	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling server information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(PORT); 
	
	// Bind the socket with the server address 
	if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
			sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	
	while (1){			// infinate loop for testing

	len = sizeof(cliaddr); //len is value/resuslt 

	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
				MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
				&len); 
	buffer[n] = '\0'; 
	//printf("Client : %s\n", buffer); 
	// after receiving an NTP request, send myPacket. Testing
	sendto(sockfd, (const void *)myPacket, sizeof(myPacket), 	// (const char *)hello
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
			len); 
	//printf("Data received. Data Sent.\n"); 
}
	//return 0; 
} 
