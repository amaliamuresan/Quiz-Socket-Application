#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define PORT 11000
#define BACKLOG_MAX_REQUESTS 10 //maximum number of requests to be put in connection queue



void server_init(int *server_fd, struct sockaddr_in address);
void accept_client_test();


int main()
{
    int server_socket;

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // connected to all interfaces, can accept connextion from any address in the same network
    address.sin_port = htons(PORT); // htons - host to network short, we make sure the data representation is correct.

    //test
    int client_socket, read_value;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}; 
    char *hello = "Hello from server"; 
    
    
    server_init(&server_socket, address);

    if ((client_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(1); 
    }

    read_value = read( client_socket , buffer, 1024); 
    printf("%s\n",buffer ); 
    send(client_socket , hello , strlen(hello) , 0 ); 
    printf("Hello message sent\n");  
    //end test

    close(client_socket);
    close(server_socket);

    return 0;
}



void server_init(int *serverfd, struct sockaddr_in address)
{

    int option = 1; // option active for sockopt

    // SOCK_STREAM - for TCP/IP connexion type
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
    
}
