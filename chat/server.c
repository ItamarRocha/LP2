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
#include <time.h>
#include <semaphore.h>

#define LENGTH_NAME 41
#define MAXLINE 1024

typedef struct Node { //implement the struct from which we are producing our nodes (aka the clients information)
    int sock;
    struct Node* prev;
    struct Node* next;
    char ip[16];
    char name[LENGTH_NAME];
} Node;

Node *newNode(int sockfd, char* ip) { //function that basically initializes our nodes (if in C++ could be a constructor)
    Node *n = (Node *)malloc( sizeof(Node) );
    n->sock = sockfd;
    n->prev = NULL;
    n->next = NULL;
    strncpy(n->ip, ip, 16);
    strncpy(n->name, "NOT DEF", 8);
    return n;
}


// Global variables
int server_sockfd = 0, client_sockfd = 0; //create both sockets
Node *initial_node, *current_node; // create the node that is going to be static (the bottom of the list) and the current that is more likely a cursor to the last element
char chat_name[100]; // name of the chat
FILE* chat_register; // log file
char current_time[50]; // global variable time
sem_t write_acess; //control the writting acess

void catch_ctrl_c_and_exit() {  //function to free all nodes after sigint in signal defined in main
    Node *cursor;
    while (initial_node != NULL) {
        printf("\nClose socketfd: %d\n", initial_node->sock);
        close(initial_node->sock); // close all socket include server_sockfd
        cursor = initial_node;
        initial_node = initial_node->next;
        free(cursor);
    }
    
    printf("\e[1;1H\e[2J");  // \e[2J clears the entire screen from top(J) to bottom(2)
    // \e[1;1H postions the cursor at row 1 column 1 of terminal
    
    printf("\n\n\n\n------");
    printf("\x1B[31mSERVER CLOSED SUCCESSFULLY\x1B[0m"); //red colored
    printf("------\n\n\n\n\n");
    fclose(chat_register);
    exit(EXIT_SUCCESS);
}

char* get_time(){
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );
  strftime(current_time, sizeof(current_time), "%d-%m-%Y-%X", timeinfo);

  return current_time; 
}

void chat(Node *n, char temp_buffer[]) { // send the message to all the other clients
    Node *cursor = initial_node->next;
    while (cursor != NULL) {
        if (n->sock != cursor->sock) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", cursor->sock, temp_buffer);
            write(cursor->sock, temp_buffer, MAXLINE); //sends the message
        }
        cursor = cursor->next; // pass to the next node and keeps sending till the last one
    }
    fprintf(chat_register,"[%s]%s\n",get_time(),temp_buffer);
}


void client_handler(void *node_client) { //function that is going to be used by the threads
    int leave_flag = 0;
    char client_name[LENGTH_NAME]; // name specified by the client
    char recv_buffer[MAXLINE];  // What was received 
    char send_buffer[MAXLINE]; // what was sent
    char welcome_message[MAXLINE];
    char error_message[MAXLINE];

    Node *n = (Node *)node_client;  // node that represents the client in this thread

    sprintf(welcome_message,"Welcome to the chat \x1B[31m[%s]\x1B[0m.",chat_name);
    
    // get the sockets name
    if (read(n->sock, client_name, LENGTH_NAME) <= 0 || strlen(client_name) >= LENGTH_NAME-1) {
        sprintf(error_message,"%s didn't input name. The max length of name admitted is 40 characters.\n", n->ip);
        printf("%s\n",error_message);
        write(n->sock,error_message,strlen(error_message));
        leave_flag = 1;
    } else {
        strncpy(n->name, client_name, LENGTH_NAME);
        printf("%s(%s)(%d) join the chatroom.\n", n->name, n->ip, n->sock);
        sprintf(send_buffer, "%s(%s) join the chatroom", n->name, n->ip);
        write(n->sock,welcome_message,strlen(welcome_message));
        sem_post(&write_acess); //write acess to the chat
        chat(n, send_buffer);
        sem_wait(&write_acess); 
    }

    
    while (1) { // normal chat
        if (leave_flag) {
            break;
        }
        int receive = read(n->sock, recv_buffer, MAXLINE);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s：%s", n->name, recv_buffer);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) { //leave flags
            printf("%s(%s)(%d) left the chatroom.\n", n->name, n->ip, n->sock);
            sprintf(send_buffer, "%s(%s) left the chatroom.", n->name, n->ip);
            leave_flag = 1;
        } else {
            printf("Error: -1\n");
            leave_flag = 1;
        }
        sem_post(&write_acess); //semaphore to acess archive
        chat(n, send_buffer);
        sem_wait(&write_acess); 
    }

    close(n->sock); // close the socket connection
    if (n == current_node) {    // remove an edge node
        current_node = n->prev;
        current_node->next = NULL;
    } else {    // remove a middle node
        n->prev->next = n->next;
        n->next->prev = n->prev;
    }
    free(n);
}

int main(int argc, char* argv[])
{
    if(argc != 3){
        printf("Wrong Pattern\n./server SERVER_PORT CHAT_NAME\n");
        exit(EXIT_FAILURE);
    }

    sem_init(&write_acess,1,0);

    int port = atoi(argv[1]); //port server
    strcpy(chat_name,argv[2]); //name of the chat

    char archive_name[MAXLINE]; //name of the archive were the conversations arer going to be recorded

    sprintf(archive_name,"%s.txt",chat_name); 
    chat_register = fopen(archive_name,"a"); //opening the archive in the main, so there is no need for the future threads to open and close it numerous times

    signal(SIGINT, catch_ctrl_c_and_exit); // catchinh the control c in order to close the program correctly

    // Create socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0); //creating the tcp socket
    if (server_sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;

    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);

    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);

    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(port);

    // Bind and Listen
    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen); //get the ip to which the socket is bound to
    if(port != ntohs(server_info.sin_port)){
        printf("This port seems to be busy. Conection could not be established. Try another one\n");
        exit(EXIT_FAILURE);
    }

    printf("Start Server %s on: %s:%d\n",chat_name, inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    // Initial linked list for clients
    initial_node = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    current_node = initial_node;

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen); //get the socket to which was connected infos
        printf("Client %s:%d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        Node *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->prev = current_node;
        current_node->next = c;
        current_node = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}