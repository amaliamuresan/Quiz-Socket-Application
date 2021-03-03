#include<stdio.h>
#include <sys/socket.h>
#include<stdlib.h>
#include <unistd.h> 
#include<string.h>
#include<arpa/inet.h>

#define SERVER_PORT 11210

int main()
{
    int clientFd;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);
    char buffer[1024] = {0};
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

    send(clientFd, Hello, strlen(Hello), 0);
    read(clientFd, buffer, 1024);
    printf("%s\n", buffer);

}



