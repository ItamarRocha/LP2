#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXLINE 1024 

int main(int argc,char*argv[]) {

	if(argc != 2){
		printf("Pattern:\n./receptor PORT\n");
		return 1;
	}

	char port[5];

	strcpy(port,argv[1]);

	int sockfd; 
	char buffer[MAXLINE]; 
	struct sockaddr_in recepaddr, emiaddr; 
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	memset(&recepaddr, 0, sizeof(recepaddr)); 
	memset(&emiaddr, 0, sizeof(emiaddr)); 
	
	recepaddr.sin_family = AF_INET; 
	recepaddr.sin_addr.s_addr = INADDR_ANY; //receive from any address
	recepaddr.sin_port = htons(atoi(port)); 
	
	if ( bind(sockfd, (const struct sockaddr *)&recepaddr, 
			sizeof(recepaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	int len, n; 

	len = sizeof(emiaddr); 

	FILE* archive = fopen("received.txt","w");
	char* end = "fim";

	while(1){
		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &emiaddr, &len);
		
		buffer[n] = '\0';
		if(!strcmp(buffer,end))//if end == buffer -> end program
			break; 

		fprintf(archive, "%s",buffer );
		printf("Received from emissor : %s\n", buffer);
	}
	
	return 0; 
} 
