typedef struct StructEmail 
{
	char* to;
	char* from;
	char* subject;
	char* body;
	char* host;
	unsigned short port;
} Email;

typedef Email* PtrEmail;

int initEmail(PtrEmail* email);
void destructEmail(PtrEmail email);