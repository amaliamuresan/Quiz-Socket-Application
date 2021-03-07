#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define PORT 11210
#define BACKLOG_MAX_REQUESTS 10 //maximum number of requests to be put in connection queue
#define true 1
#define MAX_LENGTH 4096

void server_init(int *server_fd, struct sockaddr_in address);
void *server_listener(void *argfd);
void *client_handler(void *arg);
void send_to_client(char *message,int socket);
void send_questions(char* fileName,int socket);
void send_answers(int fileNumber,int socket);

struct sockaddr_in address;
pthread_t server_listener_thread;
pthread_t client_handler_threads[1000];
int client_sockets[1000];
int clientsnrtosend;

int main()
{
    int server_socket;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // connected to all interfaces, can accept connection from any address in the same network
    address.sin_port = htons(PORT); // htons - host to network short, we make sure the data representation is correct.

    server_init(&server_socket, address);

    if(pthread_create(&server_listener_thread, NULL, server_listener, &server_socket))
    {
        fprintf(stderr, "Listener thread error\n");
    }

    if(pthread_join(server_listener_thread, NULL))
    {
        fprintf(stderr, "Listener thread join error\n");
    }
    /* 
        Client handler threads should be joined here as a precaution.
        If the threads are not closed at this point, something went wrong. 
    */
   
    
    close(server_socket);

    return 0;
}



void server_init(int *serverfd, struct sockaddr_in address)
{

    int option = 1; // option active for sockopt

    // SOCK_STREAM - for TCP/IP connection type
    *serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(*serverfd < 0)
    {
        perror("Error while creating socket");
        exit(1);
    }

    if (setsockopt(*serverfd, SOL_SOCKET, SO_REUSEADDR , &option, sizeof(option))) 
    {
        perror("Error while setting socket options");
        exit(1);
    }

    // (struct sockaddr *)&address - by convention we have to cast as sockaddr
    if(bind(*serverfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Error while binding socket");
        exit(1);
    }

    if(listen(*serverfd, BACKLOG_MAX_REQUESTS) < 0)
    {
        perror("Listen initialization not successfull");
        exit(1);
    }

    printf("Server started!\n");
    
}


void *server_listener(void *argfd)
{
    int serverfd = *((int*) argfd);
    int clientsnr=0;
    while(true)
    {
        printf("Waiting for client to connect\n");
        int client_socket,read_value;
        int addrlen = sizeof(address);
        char buffer[1024] = {0}; 
        char *hello = "Hello from server"; 

        if ((client_socket = accept(serverfd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(1); 
        }
        client_sockets[clientsnr]=client_socket;
        /* 
            Client handler thread should be created here, using the client_socket as an argument
            Also, the thread must be saved in an array
        */
       clientsnrtosend=clientsnr;
       if(pthread_create(&client_handler_threads[clientsnr],NULL,client_handler,(void *)&clientsnrtosend))
       {
           perror("Client handler thread creation error");
           exit(1);
       }
       if(pthread_detach(client_handler_threads[clientsnr]))
       {
           perror("Client handler thread detaching error");
           exit(1);
       }
       clientsnr++;
        /* Test feature until client handler is implemented */
        /*printf("Client connected\n");

        read_value = read(client_socket , buffer, 1024); 
        printf("%s\n",buffer ); 
        send(client_socket , hello , strlen(hello) , 0 ); 
        printf("Hello message sent\n");  

        close(client_socket);*/

        /* End test */
    }

}
void *client_handler(void *arg)
{
    int *client_sockp=(int *)arg;
    int clientnr=*client_sockp;
    int client_sock=client_sockets[clientnr];
    int charsread;
    char buf[1024] = {0}; 
    char exitstr[10]="exit";
    //sending stuff for testing
    char hello[100]; 
    sprintf(hello,"Hello from server, Client socket nr %d, Client NR %d",client_sock,clientnr);
    send(client_sock , hello , strlen(hello) , 0 ); 
    send(client_sock , hello , strlen(hello) , 0 ); 
    send(client_sock , hello , strlen(hello) , 0 );
    send_questions("questions.txt",client_sock);
    send_answers(1,client_sock);
    //testing end 
    printf("Hello message sent\n");  
    while(true)
    {
        charsread=read(client_sock,buf,1024);
        buf[charsread]='\0';
        printf("READ: %s\n",buf);
        if(strcmp(buf,exitstr)==0)
        {
            printf("CLOSED THREAD for client socket nr %d, Client NR %d\n",client_sock,clientnr);
            close(client_sock);
            pthread_exit(0);
        }
        if(charsread<0)
        {
            perror("Read error in client_handler");
            pthread_exit(NULL);
        }
    }
}
void send_to_client(char *message,int socket)
{
    send(socket,message,strlen(message),0);
}
void send_questions(char* fileName,int socket) //probabil nu e nevoie de 2 functii separate, poate fi o singura send_from_file
{
    char* questions;
    char* location = (char*)malloc(64);
    FILE *file;

    strcpy(location,"../Files/");
    strcat(location,fileName);
    file = fopen(location, "r");

    if(file == NULL)
    {
        perror("Error while opening the questions' file");
        exit(1);
    }

    questions = (char*)malloc(sizeof(MAX_LENGTH));
    if(questions == NULL)
    {
        perror("Error allocating the memory");
        exit(1);
    }

    char line[1024];
    while(fgets(line, MAX_LENGTH, file))
    {
        questions = (char*)realloc(questions, sizeof(questions) + MAX_LENGTH);
        if(questions == NULL)
        {
            perror("Error reallocating the memory");
            exit(1);
        }

        send_to_client(line,socket);
    }
   
}
void send_answers(int fileNumber,int socket) //functia primeste int daca este nevoie de asa ceva, dar send_questions poate fi facut sa trimita si raspunsurile
{
    char* answers;
    char* fileName = (char*)malloc(255);
    char* location = (char*)malloc(255);
    FILE *file;

    strcpy(location,"../Files/");
    sprintf(fileName,"%d",fileNumber);
    strcat(location,fileName);
    strcat(location,".txt");
    file = fopen(location, "r");

    if(file == NULL)
    {
        perror("Error while opening the answers' file");
        exit(1);
    }

    answers = (char*)malloc(sizeof(MAX_LENGTH));
    if(answers == NULL)
    {
        perror("Error allocating the memory");
        exit(1);
    }

    char line[1024];
    while(fgets(line, MAX_LENGTH, file))
    {
        answers = (char*)realloc(answers, sizeof(answers) + MAX_LENGTH);
        if(answers == NULL)
        {
            perror("Error reallocating the memory");
            exit(1);
        }

        send_to_client(line,socket);
    }
   
}