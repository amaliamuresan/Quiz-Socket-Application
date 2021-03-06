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

#define false 0
#define CLIENTS_ARRAY_SIZE 1000

int checkProtocolKey(char *message,char *key);
int extract_data_from_message(char *message,char *key);

void server_init(int *server_fd, struct sockaddr_in address);
void *server_listener(void *argfd);
void *client_handler(void *arg);
void send_to_client(char *message,int socket);
void send_from_file(char* fileName,int socket, char *keyword);
void protocol_send(char *message, char *keyword, int socket);
void getQuestion(char* fileName, int questionNr, char *message);
int getLineNumber(char* line);
void writeQuestion(int questionNr, char* answer, char* nickname);
void send_from_file_special(char* fileName,int socket, char* keyword);

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
    char protocol_key_nicknameSuccess[]="nicknameSuccess";
    char protocol_key_getQuestions[]="getQuestions";
    char protocol_key_display_data[]="displayData";
    char protocol_key_select_question[]="selectQuestion";
    char protocol_key_getQuestions_success[]="getQuestionsSuccess";
    char protocol_key_send_answer[]="sendAnswer";
    char protocol_key_question_answered[]="questionAnswered";
    char protocol_key_get_answers[]="answersGet";
    char protocol_key_answers_back[]="123UniqueTest";
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
    str2[i]='\0';
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
    char buf[2048] = {0};
    char q_number[5] = {0};
    char procMessage[2048];
    //sending stuff for testing
    char hello[100];
    char mes[1000]; // 
    sprintf(hello,"Hello from server, Client socket nr %d, Client NR %d",client_sock,clientnr);
    //testing end 
    printf("Hello message sent\n");
    
    while(true)
    {
        charsread=read(client_sock,buf,2048);
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
                    protocol_send("Success", protocol_key_nicknameSuccess, client_sock);
                    printf("NICKNAME:%s\n",clients_connected[clientnr].nickname);
                }
                else
                {
                    printf("NICKNAME NOT UNIQUE ERROR\n");
                    protocol_send("nicknameNOTunique", protocol_key_error, client_sock);
                    //send nickname error to client
                }
            }
        }

        else if(checkProtocolKey(buf,protocol_key_getQuestions))
        {
            printf("Sending queesstions\n");
            send_from_file("questions",client_sock,protocol_key_display_data);
            protocol_send("Success",protocol_key_getQuestions_success, client_sock);
        }

        else if(checkProtocolKey(buf, protocol_key_select_question))
        {
            strcpy(procMessage, buf);
            if(extract_data_from_message(procMessage,protocol_key_select_question))
            {
                int question_number;
                question_number = atoi(procMessage);
                strcpy(q_number, procMessage);
                getQuestion("questions",question_number,procMessage);
                if(strcmp(procMessage,"Question not found") == 0)
                {
                    protocol_send("questionNotFound", protocol_key_error, client_sock);
                }
                else
                {
                    protocol_send("questionIncoming", protocol_key_select_question, client_sock);
                    protocol_send(procMessage+3, protocol_key_display_data, client_sock);
                    printf("Q nunmber %s\n",q_number);
                    //send_from_file(q_number,client_sock,protocol_key_display_data);
                    protocol_send("questionSent", protocol_key_select_question, client_sock);
                }
            }
        }

        else if(checkProtocolKey(buf, protocol_key_send_answer))
        {
            strcpy(procMessage, buf);
            if(extract_data_from_message(procMessage,protocol_key_send_answer))
            {
                char nick[50];
                char question[50];
                char answer[2048];
                printf("PROC:%s\n",procMessage);
                char *token;
                token = strtok(procMessage,"\\");
                strcpy(nick,token);
                token = strtok(NULL,"\\");
                strcpy(answer,token);
                token = strtok(NULL,"\\");
                strcpy(question,token);
                writeQuestion(atoi(question), answer, nick);
                protocol_send("answered", protocol_key_question_answered, client_sock);
            }
        }

        else if(checkProtocolKey(buf,protocol_key_get_answers))
        {
            strcpy(procMessage, buf);
            if(extract_data_from_message(procMessage,protocol_key_get_answers))
            {

                send_from_file(procMessage, client_sock, protocol_key_display_data);
                protocol_send("UniqueText", protocol_key_answers_back, client_sock);
                //sleep(1);
            }
            
        }

        else if(checkProtocolKey(buf,protocol_key_exit) || strlen(buf) == 0)
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

void protocol_send(char *message, char *keyword, int socket)
{
    char message_to_send[2048];
    strcpy(message_to_send, "protocolv1.2021-");
    strcat(message_to_send, keyword);
    strcat(message_to_send,":");
    strcat(message_to_send,message);
    strcat(message_to_send,";");
    send_to_client(message_to_send, socket);
    usleep(5000);
}

void send_to_client(char *message,int socket)
{
//  messageToSend=encode();
    send(socket,message,strlen(message),0);
}
void send_from_file(char* fileName,int socket, char* keyword)
{
    char* questions;
    char* location = (char*)malloc(64);
    FILE *file;

    strcpy(location,"../Files/");
    strcat(location,fileName);
    strcat(location,".txt");
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
    char *p;
    while(fgets(line, MAX_LENGTH, file))
    {
        questions = (char*)realloc(questions, sizeof(questions) + MAX_LENGTH);
        if(questions == NULL)
        {
            perror("Error reallocating the memory");
            exit(1);
        }
        printf("%s\n",line);
        p = strchr(line,'\n');
        if(p != NULL)
            strcpy(p,p+1);
        protocol_send(line,keyword,socket);
        usleep(5000);
    }
    fclose(file);
   
}



int getLineNumber(char* line)
{
    int rez = 0;
    for(int i  = 0; i < strlen(line); i++)
    {
        if(line[i] < '0' || line[i] > '9')
        {
            break;
        }
        else
        {
            rez = rez*10 + (line[i] - '0');
        }
    }

    return rez;
}

void getQuestion(char* fileName, int questionNr, char *message)
{
    char* question;
    FILE *file;

    char* location = (char*)malloc(64);

    strcpy(location,"../Files/");
    strcat(location,fileName);
    strcat(location,".txt");
    file = fopen(location, "r");
    
    if(file == NULL)
    {
        perror("Error while opening the questions' file");
        exit(1);
    }

    question = (char*)malloc(sizeof(MAX_LENGTH));
    strcpy(question,"");
    char line[1024];
    while(fgets(line, MAX_LENGTH, file))
    {
        if(getLineNumber(line) == questionNr)
        {
            strcpy(question, line);  
            break;
        }
    }
    if(strlen(question) == 0)
        strcat(question, "Question not found");
    else
    {
        char *p;
        p = strchr(question,'\n');
        if(p != NULL)
            strcpy(p,p+1);
    }
    fclose(file);
    strcpy(message, question);
}

void writeQuestion(int questionNr, char* answer, char* nickname)
{
    char fileName[15];
    sprintf(fileName, "../Files/%d", questionNr);
    strcat(fileName, ".txt");

    FILE* file = fopen(fileName, "a");

    if(file == NULL)
    {
        perror("Error while opening the file");
        exit(1);
    }

    char* stringToWrite = (char *)malloc(strlen(answer) + strlen(nickname) + 4);
    strcpy(stringToWrite, nickname);
    strcat(stringToWrite, " : ");
    strcat(stringToWrite, answer);
    strcat(stringToWrite, "\n");
    fputs(stringToWrite, file);
    //printf("String: %s", stringToWrite);
    fclose(file);
    
}