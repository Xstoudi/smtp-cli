#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "email.h"

// REF: https://www.rfc-editor.org/errata_search.php?rfc=3696&eid=1690
#define MAX_EMAIL_ADDR_SIZE 254

// REF: https://tools.ietf.org/html/rfc2822#section-2.1.1
#define MAX_SUBJECT_SIZE 78

#define MAX_BODY_SIZE 1024

// REF: $ getconf HOST_NAME_MAX
#define MAX_HOSTNAME_SIZE 64

PtrEmail initEmail()
{
    PtrEmail email = malloc(sizeof(Email));
    email->to = (char*) calloc((MAX_EMAIL_ADDR_SIZE + 1), sizeof(char));
    email->from = (char*) calloc((MAX_EMAIL_ADDR_SIZE + 1), sizeof(char));
    email->subject = (char*) calloc((MAX_SUBJECT_SIZE + 1), sizeof(char));
    email->body = (char*) calloc((MAX_BODY_SIZE + 1), sizeof(char));
    email->host = (char*) calloc((MAX_HOSTNAME_SIZE + 1), sizeof(char));
    email->port = 25;
    return email;
}

void destructEmail(PtrEmail email)
{
    free(email->to);
    free(email->from);
    free(email->subject);
    free(email->body);
    free(email->host);
    free(email);
}