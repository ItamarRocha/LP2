#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

#define LENGTH_NAME 41
#define MAXLINE 1024

// Global variables
int sockfd = 0;
char nickname[LENGTH_NAME];
sem_t main_sem;

typedef struct ClientNode {
    int data;
    struct ClientNode* prev;
    struct ClientNode* link;
    char ip[16];
    char name[LENGTH_NAME];
} ClientList;

ClientList *newNode(int sockfd, char* ip) {
    ClientList *n = (ClientList *)malloc( sizeof(ClientList) );
    n->data = sockfd;
    n->prev = NULL;
    n->link = NULL;
    strncpy(n->ip, ip, 16);
    strncpy(n->name, "NOT DEF", 8);
    return n;
}

void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void catch_ctrl_c_and_exit(int sig) {
    sem_post(&main_sem);
}

void recv_msg_handler() {
    char receiveMessage[MAXLINE] = {};
    while (1) {
        int receive = read(sockfd, receiveMessage, MAXLINE);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            str_overwrite_stdout();
        } else if (receive == 0) {
            break;
        } else { 
            // -1 
        }
    }
}

void send_msg_handler() {
    char message[MAXLINE] = {};
    while (1) {
        str_overwrite_stdout();
        while (fgets(message, MAXLINE, stdin) != NULL) {
            str_trim_lf(message, MAXLINE);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        write(sockfd, message, MAXLINE);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

int main(int argc, char* argv[])
{
    if(argc != 4){
        printf("Wrong Pattern\n./client SERVER_IP SERVER_PORT CLIENT_NAME\n");
        exit(EXIT_FAILURE);
    }

    char IP[16];
    int port = atoi(argv[2]);

    strcpy(IP,argv[1]);

    strcpy(nickname,argv[3]);

    signal(SIGINT, catch_ctrl_c_and_exit); //func to handle the control C.

    // Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;

    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);

    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);

    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(IP);
    server_info.sin_port = htons(port);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);//get the ip to which the socket is bound to
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);//get the socket to which was connected infos

    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, strlen(nickname), 0);

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }


    sem_init(&main_sem,0,0);
    sem_wait(&main_sem);

    printf("\e[1;1H\e[2J");  // \e[2J clears the entire screen from top(J) to bottom(2)
    // \e[1;1H postions the cursor at row 1 column 1 of terminal
    printf("\n\n\n\n------");
    printf("\x1B[31mExited sucessfully\x1B[0m");
    printf("------\n\n\n\n");

    close(sockfd);
    return 0;
}