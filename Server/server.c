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
#define false 0
#define CLIENTS_ARRAY_SIZE 1000

int checkProtocolKey(char *message,char *key);
int extract_data_from_message(char *message,char *key);
void server_init(int *server_fd, struct sockaddr_in address);
void *server_listener(void *argfd);
void *client_handler(void *arg);

struct sockaddr_in address;
pthread_t server_listener_thread;
typedef struct {
    pthread_t client_handler_thread;
    int client_socket;
    int allocated;
    char nickname[100];
} client_data;
client_data clients_connected[CLIENTS_ARRAY_SIZE];

int clientsnrtosend;

//protocol define
    //protocol version
    char protocol_identifier[16]="protocolv1.2021";
    //possible keywords
    char protocol_key_exit[5]="exit";
    char protocol_key_error[6]="error";
    /*data interpretation:
        nicknameNOTunique -> Nickname is not unique
    */
    char protocol_key_nickname[9]="nickname";
    /* USAGE:
        protocol_identifier+"-"+key_something+":"+data;
    */
//protocol define

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
    int i;
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

    for(i=0;i<CLIENTS_ARRAY_SIZE;i++)
    {
        clients_connected[i].allocated=false;
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
        clients_connected[clientsnr].client_socket=client_socket;
        clients_connected[clientsnr].allocated=true;
        /* 
            Client handler thread should be created here, using the client_socket as an argument
            Also, the thread must be saved in an array
        */
       clientsnrtosend=clientsnr;
       if(pthread_create(&clients_connected[clientsnr].client_handler_thread,NULL,client_handler,(void *)&clientsnrtosend))
       {
           perror("Client handler thread creation error");
           exit(1);
       }
       if(pthread_detach(clients_connected[clientsnr].client_handler_thread))
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
int checkProtocolKey(char *message,char *key)
{
    int i,pos,ok;
    ok=true;
    for(i=0;i<strlen(protocol_identifier) && ok==true;i++)
    {
        if(message[i]!=protocol_identifier[i])
        {
            ok=false;
        }
    }
    pos=i;
    if(message[pos]!='-' && ok==true)
    {
        ok=false;
    }
    pos++;
    for(i=0;i<strlen(key) && ok==true;i++)
    {
        if(message[pos+i]!=key[i])
        {
            ok=false;
        }
    }
    pos+=i;
    if(message[pos]!=':' && ok==true)
    {
        ok=false;
    }
    return ok;
}
int extract_data_from_message(char *message,char *key)
{
    char *str=message+(strlen(protocol_identifier)+1+strlen(key)+1);
    char str2[100];
    int i=0;
    while(str[i]!=';' && i<strlen(str))
    {
        str2[i]=str[i];
        i++;
    }
    strcpy(message,str2);
    if(str[i]==';')
    {
        return true;
    }
    return false;
}
int check_unique_username(char *name)
{
    int i=0,ok=true;
    for(i=0;i<CLIENTS_ARRAY_SIZE && ok==true;i++)
    {
        if(clients_connected[i].allocated)
        {
            if(strcmp(name,clients_connected[i].nickname)==0)
            {
                ok=false;
            }
        }
    }
    return ok;
}
void *client_handler(void *arg)
{
    int *client_sockp=(int *)arg;
    int clientnr=*client_sockp;
    int client_sock=clients_connected[clientnr].client_socket;
    int charsread;
    char buf[1025] = {0}; 
    char procMessage[1025];
    //sending stuff for testing
    char hello[100];
    char mes[1000]; // 
    sprintf(hello,"Hello from server, Client socket nr %d, Client NR %d",client_sock,clientnr);
    send(client_sock , hello , strlen(hello) , 0 ); 
    send(client_sock , hello , strlen(hello) , 0 ); 
    send(client_sock , hello , strlen(hello) , 0 );
    //testing end 
    printf("Hello message sent\n");
    
    while(true)
    {
        charsread=read(client_sock,buf,1024);
        buf[charsread]='\0';
        printf("READ: %s\n",buf);
        if(checkProtocolKey(buf,protocol_key_nickname))
        {
            strcpy(procMessage,buf);
            if(extract_data_from_message(procMessage,protocol_key_nickname))
            {
                if(check_unique_username(procMessage))
                {
                    strcpy(clients_connected[clientnr].nickname,procMessage);
                    printf("NICKNAME:%s\n",clients_connected[clientnr].nickname);
                }
                else
                {
                    printf("NICKNAME NOT UNIQUE ERROR\n");
                    //send nickname error to client
                }
            }
        }
        if(checkProtocolKey(buf,protocol_key_exit))
        {
            printf("CLOSED THREAD for client socket nr %d, Client NR %d\n",client_sock,clientnr);
            clients_connected[clientnr].allocated=false;
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