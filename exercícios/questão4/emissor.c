#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

int main(int argc, char* argv[]) { 
	if(argc != 4){
		printf("Pattern :\n./emissor archive.txt X.X.X.X PORT\n");
		return 1;
	}

	char archive_name[100] = "";
	strcpy(archive_name,argv[1]);

	char IP[20] = ""; //IP of the machine
	strcpy(IP,argv[2]);

	int PORT = atoi(argv[3]); //PORT

	int sockfd;  
	struct sockaddr_in recepaddr; 

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 

	memset(&recepaddr, 0, sizeof(recepaddr)); 
	
	recepaddr.sin_family = AF_INET; 
	recepaddr.sin_port = htons(PORT); 
	recepaddr.sin_addr.s_addr = inet_addr(IP); 
	
	char string[1024];
	FILE* archive = fopen(archive_name,"r");

	while(fgets(string,1024,archive) != NULL){
		printf("Sent : %s\n",string);
		sendto(sockfd, (const char *)string, strlen(string), MSG_CONFIRM, (const struct sockaddr *) &recepaddr, sizeof(recepaddr)); 
	}
	//send last message to comunicate end of the transmission
	sendto(sockfd, (const char *)"fim", strlen("fim"), MSG_CONFIRM, (const struct sockaddr *) &recepaddr, sizeof(recepaddr)); 
	fclose(archive);
	close(sockfd); 
	return 0; 
} 
