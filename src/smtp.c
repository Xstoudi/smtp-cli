#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int initSocket()
{
    int sock = 0;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket init error.");
        exit(-1);
    }
    printf("\nSocket created.");
    return sock;
}

void prepareServAddr(char* host, int port, struct sockaddr_in* serv_addr)
{
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(port);
    if(inet_pton(AF_INET, host, &serv_addr->sin_addr) <= 0)
    {
        printf("\nAddress invalid or not supported.");
        exit(-1);
    }
    printf("\nHost and port are valid.");
}

void smtpConnect(int sock, struct sockaddr_in* serv_addr)
{
    if(connect(sock, (struct sockaddr*)serv_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nConnection failed.");
        exit(-1);
    }
    printf("\nConnected to server");
    fflush(stdout);
}

void smtpReceive(int sock, char buffer[2048])
{
    int n = 0;
    if((n = read(sock, buffer, sizeof(char) * 2048, 0)) < 0)
    {
        printf("\nFail on receive.");
        exit(-1);
    }
    if(n == 0)
    {
        printf("\nServer disconnected");
        exit(-1);
    }
    printf("\n\"%s\"", buffer);
    fflush(stdout);
}

void smtpSend(int sock, char* buffer)
{
    send(sock, buffer, strlen(buffer), 0);
}

int extractResponseCode(char buffer[2048])
{
    char strCode[3] = "";
    strncpy(strCode, buffer, 3);
    return atoi(strCode);
}

char* buildCommandWithParam(char* field, char* value)
{
    char* builded = calloc((strlen(field) + strlen(value) + 5), sizeof(char));
    strcat(builded, field);
    strcat(builded, ": <");
    strcat(builded, value);
    strcat(builded, ">\n");
    return builded;
}

char* buildData(char* subject, char* body)
{
    char* subjectHeader = "Subject: ";
    char* finalPoint = "\n.\n";
    char* data = calloc(strlen(subject) + strlen(body) + strlen(subjectHeader) + 1 + strlen(finalPoint), sizeof(char));
    strcat(data, subjectHeader);
    strcat(data, subject);
    strcat(data, "\n");
    strcat(data, body);
    strcat(data, finalPoint);
    return data;
}