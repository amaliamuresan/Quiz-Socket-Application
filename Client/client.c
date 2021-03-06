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
#define false 0

int checkProtocolKey(char *message,char *key);
int extract_data_from_message(char *message,char *key);
void *client_receive(void *arg);
void *send_to_server(void *arg,int socket);

pthread_t client_receive_thread;

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
        exit(1);
    }
    //testing
    char mes[1000];
    scanf("%s",scanned);
    strcpy(mes,"protocolv1.2021-nickname:");
    strcat(mes,scanned);
    strcat(mes,";");
    send(clientFd,mes,strlen(mes),0);
    strcpy(scanned,"38qfabb39");
    send(clientFd,scanned,strlen(scanned),0);
    strcpy(scanned,"G$G44wwg");
    send(clientFd,scanned,strlen(scanned),0);
    send_to_server("\nceva\n",clientFd);
    scanf("%s",scanned);//supposed to enter "exit"
    strcpy(mes,"protocolv1.2021-exit:;");
    send(clientFd,mes,strlen(mes),0);
    printf("CLOSED CLIENT\n");
    close(clientFd);
    exit(0);
    //testing
    if(pthread_join(client_receive_thread,NULL))
    {
        perror("Client receive thread join error");
        exit(1);
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
void *client_receive(void *arg)
{
    char buf[1025] = {0};
    char procMessage[1025];
    int readqt;
    int *clientfdp=(int *)arg;
    int clientfd=*clientfdp;
    while(true)
    {
        readqt=read(clientfd, buf, 1024);
        buf[readqt]='\0';
        printf("READ:%s\n", buf);
        if(checkProtocolKey(buf,protocol_key_error))
        {
            strcpy(procMessage,buf);
            if(extract_data_from_message(procMessage,protocol_key_error))
            {
                if(strcmp(procMessage,"nicknameNOTunique")==0)
                {
                    //restart nickname request
                    printf("NICKNAME NOT UNIQUE\n");
                }
            }
        }
        if(readqt<0)
        {
            perror("Read error in client_receive");
            pthread_exit(NULL);
        }
    }
}

void *send_to_server(void * arg,int socket){
    char* str=(char*)arg;
    send(socket,str,strlen(str),0);
}



