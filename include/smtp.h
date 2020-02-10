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

int initSocket();
int prepareServAddr(char* host, int port, struct sockaddr_in* serv_addr);
int smtpConnect(int sock, struct sockaddr_in* serv_addr);
int smtpReceive(int sock, char buffer[2048]);
void smtpSend(int sock, char* buffer);
void extractResponse(char buffer[2048], int* responseCode);
char* buildCommandWithParam(char* field, char* value);
char* buildData(char* subject, char* body);