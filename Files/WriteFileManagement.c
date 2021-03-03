#include<stdio.h>
#include<stdlib.h>
#include<string.h>

void writeQuestion(int questionNr, char* answer, char* nickname)
{
    char fileName[15];
    sprintf(fileName, "%d", questionNr);
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
    printf("String: %s", stringToWrite);
    
}

//test
int main()
{
    writeQuestion(1, "I dont know", "Jon");
    writeQuestion(2, "A variable", "Ana");

    return 0;
}