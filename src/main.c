#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "argp.h"
#include "arpa/inet.h"

#include "email.h"
#include "smtp.h"

int parse_opt(int key, char* arg, struct argp_state* state)
{
    FILE* file = NULL;
    PtrEmail email = state->input;
 //   printf("\n%c <--> %s", key, arg);
    switch(key)
    {
        case 't':
            email->to = arg;
            break;
        case 'f':
            email->from = arg;
            break;
        case 's':
            email->subject = arg;
            break;
        case 'b':
            file = fopen(arg, "r");
            if(file == NULL)
            {
                printf("\n%s not found.", arg);
                exit(-1);
            }
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);

            printf("\n%i", size);

            char* fileContent = malloc(sizeof(char) * size);
            for(int i = 0; i < size; i++)
            {
                int character = fgetc(file);
                fileContent[i] = character;
                if(character == EOF)
                {
                    break;
                }
            }
            email->body = fileContent;
            printf("\n%s", email->body);
            break;
        case 'h':
            email->host = arg;
            break;
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

void clearBuffer(char* buffer)
{
    buffer = memset(buffer, 0, sizeof(char) * 2048);
}

int main(int argc, char* argv[])
{
    // Init email
    PtrEmail email = initEmail();

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
    argp_parse(&argp, argc, argv, ARGP_IN_ORDER, 0, email);

    printf("\nTo: %s\nFrom: %s\nSubject: %s\nBody: %s\nHost: %s\nPort: %i", email->to, email->from, email->subject, email->body, email->host, email->port);
    
    // Port argument is optional
    if(email->port == 0)
    {
        email->port = 25;
    }

    SMTPState state = CONNECT;
    int sock = 0;
    char buffer[2048] = "";

    while(true)
    {
        printf("\n--> Current State : %i", state);
        fflush(stdout);
        switch(state)
        {
            case CONNECT:
                sock = initSocket();
                struct sockaddr_in serv_addr;
                prepareServAddr(email->host, email->port, &serv_addr);
                smtpConnect(sock, &serv_addr);

                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 220)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 1 ---");
                    exit(-1);
                }
                break;
            case HELLO:
                smtpSend(sock, "EHLO client\n");

                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 250)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 2 ---");
                    exit(-1);
                }
                break;
            case MAIL_FROM:
                smtpSend(sock, buildCommandWithParam("MAIL FROM", email->from));
                
                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 250)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 3 ---");
                    exit(-1);
                }
                break;
            case RCPT_TO:
                smtpSend(sock, buildCommandWithParam("RCPT TO", email->to));

                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 250)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 4 ---");
                    exit(-1);
                }
                break;
            case DATA:
                smtpSend(sock, "DATA\n");
                
                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 354)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 5 ---");
                    exit(-1);
                }
                break;
            case CONTENT:
                smtpSend(sock, buildData(email->subject, email->body));

                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 250)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 6 ---");
                    exit(-1);
                }
                break;
            case QUIT:
                smtpSend(sock, "QUIT\n");

                clearBuffer(buffer);
                smtpReceive(sock, buffer);
                if(extractResponseCode(buffer) == 221)
                {
                    state++;
                }
                else
                {
                    printf("--- ERROR TO HANDLE 7");
                    exit(-1);
                }
                break;
            case EXIT:
                printf("--- EXIT TO HANDLE ---");
                return 0;
        }
    }
}
