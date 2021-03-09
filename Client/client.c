#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include<sys/wait.h>

#define SERVER_PORT 11210
#define true 1
#define false 0

int state=0;
// state 0 - nickname
// state 1 - choose question
// state 2 - answer question
// state 3 - view answered question

int checkProtocolKey(char *message,char *key);
int extract_data_from_message(char *message,char *key);
void *client_receive(void *arg);
void send_to_server(char * message,int socket);
void protocol_send(char *message, char *keyword, int socket);

pthread_t client_receive_thread;
int nickname_set = 0;
char nickname[100];
int lock = 0;
char selected_question[5];
char nickname_message[50] = "Enter nickname!";

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
    char protocol_key_nicknameSuccess[]="nicknameSuccess";
    char protocol_key_getQuestions[]="getQuestions";
    char protocol_key_display_data[]="displayData";
    char protocol_key_select_question[]="selectQuestion";
    char protocol_key_getQuestions_success[]="getQuestionsSuccess";
    char protocol_key_send_answer[]="sendAnswer";
    /* USAGE:
        protocol_identifier+"-"+key_something+":"+data;
    */
//protocol define

int main()
{
    char answer[2048];
    int clientFd;
    struct sockaddr_in address;
    size_t size = 0;
    char *ans;
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

    while(true)
    {
        lock = 1;
        if(!nickname_set && state == 0)
        {
            printf("%s\n", nickname_message);
            scanf("%s",scanned);
            protocol_send(scanned, protocol_key_nickname, clientFd);
            strcpy(nickname,scanned);
            usleep(5000);
        }
        else if(state == 1)
        {
            usleep(5000);
            printf("Select question: ");
            scanf("%s", scanned);
            strcpy(selected_question,scanned);
            protocol_send(scanned, protocol_key_select_question, clientFd);
        }
        else if(state == 2)
        {
            getc(stdin);
            getline(&ans, &size, stdin);
            ans[strlen(ans)-1]='\0';
            printf("AM SCANAT: %s\n",ans);
            
            strcpy(answer,nickname);
            strcat(answer," : ");
            strcat(answer,ans);
            strcat(answer,"\\");
            strcat(answer,selected_question);
            protocol_send(answer, protocol_key_send_answer, clientFd);
        }

        while(lock){};
        usleep(5000);
    }

    /*
    scanf("%s",scanned);//supposed to enter "exit"
    strcpy(mes,"protocolv1.2021-exit:;");
    send(clientFd,mes,strlen(mes),0);
    printf("CLOSED CLIENT\n");
    close(clientFd);
    exit(0);

    */
    //testing
    if(pthread_join(client_receive_thread,NULL))
    {
        perror("Client receive thread join error");
        exit(1);
    }
    

}

void *client_receive(void *arg)
{
    char buf[2048] = {0};
    char procMessage[2048];
    int readqt;
    int *clientfdp=(int *)arg;
    int clientfd=*clientfdp;
    while(true)
    {
        readqt=read(clientfd, buf, 2048);
        buf[readqt]='\0';
        
        if(checkProtocolKey(buf,protocol_key_error))
        {
            //printf("READ:%s\n", buf);
            strcpy(procMessage,buf);
            //printf("MYPROC:%s\n",procMessage);
            if(extract_data_from_message(procMessage,protocol_key_error))
            {
                
                if(strcmp(procMessage,"nicknameNOTunique")==0)
                {
                    //restart nickname request
                    strcpy(nickname_message,"Enter another nickname!");
                    system("clear");
                    //printf("%s\n",procMessage);
                    printf("Nickname \"%s\" is already in use!\n", nickname);
                }
                else if(strcmp(procMessage, "questionNotFound") == 0)
                {
                    printf("Not a valid selection! Select a valid number!\n");
                }
            }
        }
        else if(checkProtocolKey(buf,protocol_key_nicknameSuccess))
        {
            nickname_set = 1;
            system("clear");
            printf("Welcome, %s!\n\n",nickname);
            printf("Choose a question from this list:\n");
            protocol_send("questions", protocol_key_getQuestions, clientfd);
            
        }
        else if(checkProtocolKey(buf,protocol_key_getQuestions_success))
        {
            state=1;
            lock=0;
        }
        else if(checkProtocolKey(buf,protocol_key_display_data))
        {
            //printf("%s\n",buf);
            strcpy(procMessage, buf);
            if(extract_data_from_message(procMessage,protocol_key_display_data))
            {
                printf("%s\n",procMessage);
            }
        }
        else if(checkProtocolKey(buf,protocol_key_select_question))
        {
            //printf("DATA: %s\n",buf);
            strcpy(procMessage, buf);
            if(extract_data_from_message(procMessage,protocol_key_select_question))
            {
                if(strcmp(procMessage,"questionIncoming"))
                {
                    
                }
                else if(strcmp(procMessage,"questionSent"))
                {
                    printf("\nWrite your answer for the next question: \n");
                    state = 2;
                    lock = 0;
                }
                else
                {
                    printf("%s\n",procMessage);
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
    char str2[2048];
    int i=0;
    while(str[i]!=';' && i<strlen(str))
    {
        str2[i]=str[i];
        i++;
    }
    str2[i]='\0';
    strcpy(message,str2);
    //printf("MYPROC2:%s\n",message);
    if(str[i]==';')
    {
        return true;
    }
    return false;
}

void protocol_send(char *message, char *keyword, int socket)
{
    char message_to_send[2048];
    strcpy(message_to_send, "protocolv1.2021-");
    strcat(message_to_send, keyword);
    strcat(message_to_send,":");
    strcat(message_to_send,message);
    strcat(message_to_send,";");
    send_to_server(message_to_send, socket);
}

void send_to_server(char * message,int socket){
    send(socket,message,strlen(message),0);
}