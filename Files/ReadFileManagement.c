#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_LENGTH 4096

char* getAllQuestions(char* fileName)
{
    char* questions;
    FILE *file;

    if(file == NULL)
    {
        perror("Error while opening the questions' file");
        exit(1);
    }

    file = fopen(fileName, "r");

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

        strcat(questions, line);  
    }

     return questions;     
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

char* getQuestion(char* fileName, int questionNr)
{
    char* question;
    FILE *file;

    file = fopen(fileName, "r");
    
    if(file == NULL)
    {
        perror("Error while opening the questions' file");
        exit(1);
    }

    question = (char*)malloc(sizeof(MAX_LENGTH));

    char line[1024];
    while(fgets(line, MAX_LENGTH, file))
    {
        if(getLineNumber(line) == questionNr)
        {
            strcat(question, line);  
            break;
        }
    }
    if(strcmp(question, "") == 0)
        strcat(question, "Question not found\n");
    
    return question;

}

//test
int main()
{
    printf("%s", getAllQuestions("questions.txt"));
    printf("Question: %s", getQuestion("questions.txt", 2));
}

