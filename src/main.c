#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "email.h"
#include "smtp.h"
#include "utils.h"
#include "dns.h"

int parse_opt(int key, char* arg, struct argp_state* state)
{
    FILE* file = NULL;
    PtrEmail email = state->input;
    switch(key)
    {
        case 't':
            strcpy(email->to, arg);
            break;
        case 'f':
            strcpy(email->from, arg);
            break;
        case 's':
            strcpy(email->subject, arg);
            break;
        case 'b':
            file = fopen(arg, "r");
            if(file == NULL)
            {
                printf("\n%s not found.", arg);
                return 1;
            }
            if(fseek(file, 0, SEEK_END) != 0)
            {
                printf("\nFail to seek file size.");
                return 1;
            }
            int size = ftell(file);
            if(fseek(file, 0, SEEK_SET) != 0)
            {
                printf("\nFail to seek file size.");
                return 1;
            }

            char* fileContent = calloc(size + 1, sizeof(char));
            if(fileContent == NULL)
            {
                printf("\nFail to allocate memory for file content.");
                return 1;
            }
            for(int i = 0; i < size; i++)
            {
                int character = fgetc(file);
                fileContent[i] = character;
                if(character == EOF)
                {
                    break;
                }
            }
            strcpy(email->body, fileContent);
            if(fclose(file) != 0)
            {
                printf("\nFail to close file content stream.");
                return 1;
            }
            free(fileContent);
            break;
        case 'h':
            {
                char* ipToCheck = calloc(strlen(arg) + 1, sizeof(char));
                strcpy(ipToCheck, arg);
                if(isValidIP(arg) == true)
                {
                    strcpy(email->host, ipToCheck);
                }
                else
                {
                    char* ip = calloc(16, sizeof(char));
                    if(hostnameToIP(ipToCheck, ip) != 0)
                    {
                        printf("\nFail to parse host.");
                        return 1;
                    }
                    strcpy(email->host, ip);
                    free(ip);
                }
                free(ipToCheck);
                break;
            }
        case 'p':
            email->port = atoi(arg);
            break;
        case ARGP_KEY_SUCCESS:
            if(email->to[0] == '\0'
                || email->from[0] == '\0'
                || email->subject[0] == '\0'
                || email->body[0] == '\0'
                || email->host[0] == '\0')
            {
                printf("Missing argument. Use --help to show usage.");
                return 1;
            }
            break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    // Init email
    PtrEmail email;
    if(initEmail(&email) != 0)
    {
        printf("\nFail to init email.");
        return 1;
    }

    // Parsing
    struct argp_option options[] = 
    {
        { "to", 't', "str", 0, "Recipient of the email.", 0 },
        { "from", 'f', "str", 0, "Sender of the email.", 0 },
        { "subject", 's', "str", 0, "Subject of the email.", 0 },
        { "body", 'b', "str", 0, "Filename containing email body.", 0 },
        { "host", 'h', "str", 0, "SMTP server host or ip address.", 0 },
        { "port", 'p', "int", 0, "SMTP server port (default 25).", 0 },
        { NULL }
    };

    struct argp argp = { options, parse_opt, NULL, NULL, NULL, NULL, NULL };
    if(argp_parse(&argp, argc, argv, ARGP_IN_ORDER, 0, email) != 0)
    {
        printf("\nFail to parse command line arguments.");
        return 1;
    }

    printf("\nTo: %s\nFrom: %s\nSubject: %s\nBody: %s\nHost: %s\nPort: %i", email->to, email->from, email->subject, email->body, email->host, email->port);

    // Port argument is optional
    if(email->port == 0)
    {
        email->port = 25;
    }

    SMTPState state = CONNECT;
    int sock = 0;
    char buffer[2048] = "";
    char* toSend;
    int responseCode = 0;

    bool continueAutomataLoop = true;
    while(continueAutomataLoop == true)
    {
        switch(state)
        {
            case CONNECT:
                sock = initSocket();
                if(sock < 0)
                {
                    printf("\nSocket initialization error.");
                    state = EXIT;
                    continue;
                }
                printf("\nSuccessfully initializated socket.");

                struct sockaddr_in serv_addr;
                if(prepareServAddr(email->host, email->port, &serv_addr) <= 0)
                {
                    printf("\nFailed to prepare servaddr.");
                    state = EXIT;
                    continue;
                }
                printf("\nSuccessfully prepared servaddr.");

                if(smtpConnect(sock, &serv_addr) < 0)
                {
                    printf("\nFailed to connect to SMTP server.");
                    state = EXIT;
                    continue;
                }
                printf("\nSuccessfully connected to SMTP server.");
                fflush(stdout);

                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }
                
                extractResponse(buffer, &responseCode);
                if(responseCode == 220)
                {
                    state++;
                }
                else if(responseCode == 554)
                {
                    printf("\nNo SMTP service here: %s:%i", email->host, email->port);
                    state = EXIT;
                    continue;
                }
                else
                {
                    state = EXIT;
                    continue;
                }
                break;
            case HELLO:
                smtpSend(sock, "HELO client\r\n");

                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }

                extractResponse(buffer, &responseCode);
                if(responseCode == 250)
                {
                    state++;
                }
                else
                {
                    state = EXIT;
                }
                break;
            case MAIL_FROM:
                toSend = buildCommandWithParam("MAIL FROM", email->from);
                smtpSend(sock, toSend);
                free(toSend);

                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }

                extractResponse(buffer, &responseCode);
                if(responseCode == 250)
                {
                    state++;
                }
                else
                {
                    state = EXIT;
                }
                break;
            case RCPT_TO:
                toSend = buildCommandWithParam("RCPT TO", email->to);
                smtpSend(sock, toSend);
                free(toSend);

                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }

                extractResponse(buffer, &responseCode);
                if(responseCode == 250 || responseCode == 251)
                {
                    state++;
                }
                else if(responseCode == 450)
                {
                    printf("We're greylisted. Will retry in 60 seconds.");
                    fflush(stdout);
                    state = HELLO;
                    sleep(60);
                }
                else
                {
                    state = EXIT;
                }
                break;
            case DATA:
                smtpSend(sock, "DATA\r\n");
                
                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }

                extractResponse(buffer, &responseCode);
                if(responseCode == 354)
                {
                    state++;
                }
                else
                {
                    state = EXIT;
                }
                break;
            case CONTENT:
                toSend = buildData(email->subject, email->body);
                smtpSend(sock, toSend);
                free(toSend);

                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }

                extractResponse(buffer, &responseCode);
                if(responseCode == 250)
                {
                    state++;
                }
                else
                {
                    state = EXIT;
                }
                break;
            case QUIT:
                smtpSend(sock, "QUIT\r\n");

                if(smtpReceive(sock, buffer) <= 0)
                {
                    state = EXIT;
                    continue;
                }

                extractResponse(buffer, &responseCode);
                if(responseCode == 221)
                {
                    state++;
                }
                else
                {
                    state = EXIT;
                }
                break;
            case EXIT:
                continueAutomataLoop = false;
                break;
            default:
                printf("\nInvalid state: %i", state);
                continueAutomataLoop = false;
        }
    }

    destructEmail(email);

    return 0;
}
