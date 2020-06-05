#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

FILE* tcpConnect(const char *hostname, const char *port)
{
    FILE* f = NULL;
    int s;
    struct addrinfo hints;
    struct addrinfo* result;
    struct addrinfo* rp;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if(getaddrinfo(hostname, port, &hints, &result) != 0) {
        return NULL;
    } else {
        for(rp = result; rp != NULL; rp = rp->ai_next) {
            char ipname[INET_ADDRSTRLEN];
            char servicename[6];
            if(!getnameinfo(
                    rp->ai_addr,
                    rp->ai_addrlen,
                    ipname,
                    sizeof(ipname),
                    servicename,
                    sizeof(servicename),
                    NI_NUMERICHOST|NI_NUMERICSERV
            )) {
                if((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1){
                    return NULL;
                } else
                {
                    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1)
                    {
                        if ((f = fdopen(s, "r+")))
                        {
                            break;
                        }
                    } else {
                        return NULL;
                    }
                }
                close(s);
            }
        }
        freeaddrinfo(result);
    }
    return f;
}

int smtpReceive(FILE* file, char buffer[2048])
{
    if(fgets(buffer, 2048, file) == NULL){
        return 1;
    }
    return 0;
}

void smtpSend(FILE* file, char* buffer)
{
    if(getenv("DEBUG"))
    {
        printf("\nE: %s", buffer);
    }
    else
    {
        printf(".");
    }
    fputs(buffer, file);
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