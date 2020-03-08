#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int initSocket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

int prepareServAddr(char* host, int port, struct sockaddr_in* servAddr)
{
    servAddr->sin_family = AF_INET;
    servAddr->sin_port = htons(port);
    return inet_pton(AF_INET, host, &servAddr->sin_addr);
}

int smtpConnect(int sock, struct sockaddr_in* servAddr)
{
    return connect(sock, (struct sockaddr*)servAddr, sizeof(struct sockaddr));
}

int smtpReceive(int sock, char buffer[2048])
{
    buffer = memset(buffer, 0, sizeof(char) * 2048);
    int result = read(sock, buffer, sizeof(char) * 2048);
    if(result <= 0)
    {
        printf("\nDisconnected from server.");
    }
    return result;
}

void smtpSend(int sock, char* buffer)
{
    if(getenv("DEBUG"))
    {
        printf("\nE: %s", buffer);
    }
    else
    {
        printf(".");
    }
    send(sock, buffer, strlen(buffer), 0);
}

void extractResponse(char buffer[2048], int* responseCode)
{
    char strCode[3] = "";
    strncpy(strCode, buffer, 3);
    *responseCode = atoi(strCode);
    int length = strlen(buffer) - 5;
    if(length > 0)
    {
        memcpy(buffer, buffer + 4, length);
        buffer[length] = '\0';
    }
    else
    {
        buffer = memset(buffer, 0, sizeof(char) * 2048);
    }
    if(getenv("DEBUG"))
    {
        printf("\nR: %i - %s", *responseCode, buffer);
    }
    else
    {
        printf(".");
    }
}

char* buildCommandWithParam(char* field, char* value)
{
    char* builded = calloc((strlen(field) + strlen(value) + 6), sizeof(char));
    strcat(builded, field);
    strcat(builded, ":<");
    strcat(builded, value);
    strcat(builded, ">\r\n");
    return builded;
}

char* buildData(char* subject, char* body)
{
    char* subjectHeader = "Subject: ";
    char* finalPoint = "\r\n.\r\n";
    char* data = calloc(strlen(subject) + strlen(body) + strlen(subjectHeader) + 1 + strlen(finalPoint) + 1, sizeof(char));
    strcat(data, subjectHeader);
    strcat(data, subject);
    strcat(data, "\n");
    strcat(data, body);
    strcat(data, finalPoint);
    return data;
}