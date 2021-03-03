#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_PORT 11210
#define true 1

void *client_receive(void *arg);

pthread_t client_receive_thread;

int main()
{
    int clientFd;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);
    
    address.sin_addr.s_addr = INADDR_ANY;

    if((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error socket creation");
        exit(1);
    }

    if(connect(clientFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Error connecting to server");
        exit(1);
    }

    char* Hello = "Hello from the client side";
    char scanned[1024];
    char exitstr[10]="exit";

    send(clientFd, Hello, strlen(Hello), 0);
    
    if(pthread_create(&client_receive_thread,NULL,client_receive,(void *)&clientFd))
    {
        perror("Client receive thread creation error");
    }
    while(true)
    {
        printf("What to send:");
        scanf("%s",scanned);
        printf("\n");
        send(clientFd,scanned,strlen(scanned),0);
        if(strcmp(scanned,exitstr)==0)
        {
            printf("CLOSED CLIENT\n");
            exit(0);
        }
    }
    if(pthread_join(client_receive_thread,NULL))
    {
        perror("Client receive thread join error");
    }
    close(clientFd);

}
void *client_receive(void *arg)
{
    char buf[1025] = {0};
    int readqt;
    int *clientfdp=(int *)arg;
    int clientfd=*clientfdp;
    while(true)
    {
        readqt=read(clientfd, buf, 1024);
        buf[readqt]='\0';
        printf("READ:%s\n", buf);
        if(readqt<0)
        {
            perror("Read error in client_receive");
            pthread_exit(NULL);
        }
    }
}



