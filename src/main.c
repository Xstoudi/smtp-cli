#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include "email.h"
#include "smtp.h"
#include "dns.h"

int parse_opt(int key, char* arg, struct argp_state* state)
{
    FILE* file = NULL;
    PtrEmail email = state->input;
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
            email->body = fileContent;
            if(fclose(file) != 0)
            {
                printf("\nFail to close file content stream.");
                return 1;
            }
            break;
        case 'h':
            email->host = arg;
            break;
        case 'p':
            email->port = arg;
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
    int exitCode = 0;

    // Init email
    PtrEmail email;
    if(initEmail(&email) != 0)
    {
        printf("\nFail to init email.");
        return 1;
    }

    argp_program_version = "1.0.0";
    argp_program_bug_address = "xavier.stouder@he-arc.ch";

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
        exitCode = 1;
    }
    else
    {
        // Port argument is optional
        if(email->port[0] == '\0')
        {
            email->port = "25";
        }

        if(getenv("DEBUG"))
        {
            printf("\nTo: %s\nFrom: %s\nSubject: %s\nBody: %s\nHost: %s\nPort: %i", email->to, email->from, email->subject, email->body, email->host, atoi(email->port));
        }

        SMTPState state = CONNECT;
        FILE* f;
        char buffer[2048] = "";
        char* toSend;


        bool continueAutomataLoop = true;
        while(continueAutomataLoop == true)
        {
            switch(state)
            {
                case CONNECT:
                    f = tcpConnect(email->host, email->port);
                    if(f == NULL) {
                        state = CRITICAL_ERROR;
                        continue;
                    }

                    handleState(f, buffer, &state);
                    break;
                case HELLO:
                    smtpSend(f, "HELO client\r\n");

                    handleState(f, buffer, &state);
                    break;
                case MAIL_FROM:
                    toSend = buildCommandWithParam("MAIL FROM", email->from);
                    smtpSend(f, toSend);
                    free(toSend);

                    handleState(f, buffer, &state);
                    break;
                case RCPT_TO:
                    toSend = buildCommandWithParam("RCPT TO", email->to);
                    smtpSend(f, toSend);
                    free(toSend);

                    handleState(f, buffer, &state);
                    break;
                case DATA:
                    smtpSend(f, "DATA\r\n");

                    handleState(f, buffer, &state);
                    break;
                case CONTENT:
                    toSend = buildData(email->subject, email->body);
                    smtpSend(f, toSend);
                    free(toSend);

                    handleState(f, buffer, &state);
                    break;
                case QUIT:
                    smtpSend(f, "QUIT\r\n");

                    handleState(f, buffer, &state);
                    break;
                case CRITICAL_ERROR:
                    state = EXIT;
                    exitCode = 1;
                    break;
                case EXIT:
                    continueAutomataLoop = false;
                    break;
                default:
                    printf("\nInvalid state: %i", state);
                    state = CRITICAL_ERROR;
            }
        }
    }

    destructEmail(email);

    return exitCode;
}
