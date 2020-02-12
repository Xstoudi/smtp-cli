/* SMTP protocol automata type. */
typedef enum EnumSMTPState {
    CONNECT,
    HELLO,
    MAIL_FROM,
    RCPT_TO,
    DATA,
    CONTENT,
    QUIT,
    EXIT
} SMTPState;

/* Socket initialization. Returns 0 if successful. */
int initSocket();
/* Prepare server address structure. Returns 0 if successful. */
int prepareServAddr(char* host, int port, struct sockaddr_in* serv_addr);
/* Connect to SMTP server. Returns 0 if successful. */
int smtpConnect(int sock, struct sockaddr_in* serv_addr);
/* Receive datas from SMTP socket. Returns N bytes received. */
int smtpReceive(int sock, char buffer[2048]);
/* Send datas in the SMTP socket. */
void smtpSend(int sock, char* buffer);
/* Extract response code and message from SMTP response. */
void extractResponse(char buffer[2048], int* responseCode);
/* Build SMTP command with parameter. Returns the full command. */
char* buildCommandWithParam(char* field, char* value);
/* Correctly build content to be sent. Returns the builded content. */
char* buildData(char* subject, char* body);