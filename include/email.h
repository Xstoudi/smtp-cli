/* Email structure type. */
typedef struct StructEmail 
{
	char* to;
	char* from;
	char* subject;
	char* body;
	char* host;
	char* port;
} Email;

/* Email structure pointer. */
typedef Email* PtrEmail;

/* Init email by passing a pointer. Returns 0 if successful. */
int initEmail(PtrEmail* email);
/* Destruct email and free associated pointers. */
void destructEmail(PtrEmail email);