/* SMTP protocol automata type. */
typedef enum EnumSMTPState {
    CONNECT,
    HELLO,
    MAIL_FROM,
    RCPT_TO,
    DATA,
    CONTENT,
    QUIT,
    EXIT,
    CRITICAL_ERROR
} SMTPState;

/* Socket initialization. Returns 0 if successful. */
int initSocket();
/* Prepare server address structure. Returns 0 if successful. */
int prepareServAddr(char* host, int port, struct sockaddr_in* servAddr);
/* Connect to SMTP server. Returns 0 if successful. */
FILE* tcpConnect(const char *hostname, const char *port);
/* Receive datas from SMTP socket. Returns N bytes received. */
int smtpReceive(FILE* file, char buffer[2048]);
/* Send datas in the SMTP socket. */
void smtpSend(FILE* file, char* buffer);
/* Extract response code and message from SMTP response. */
void extractResponse(char buffer[2048], int* responseCode);
/* Build SMTP command with parameter. Returns the full command. */
char* buildCommandWithParam(char* field, char* value);
/* Correctly build content to be sent. Returns the builded content. */
char* buildData(char* subject, char* body);

void handleState(FILE* f, char buffer[1024], SMTPState* state);