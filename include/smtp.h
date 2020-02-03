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
void prepareServAddr(char* host, int port, struct sockaddr_in* serv_addr);
void smtpConnect(int sock, struct sockaddr_in* serv_addr);
void smtpReceive(int sock, char buffer[2048]);
void smtpSend(int sock, char* buffer);
int extractResponseCode(char buffer[2048]);
char* buildCommandWithParam(char* field, char* value);
char* buildData(char* subject, char* body);