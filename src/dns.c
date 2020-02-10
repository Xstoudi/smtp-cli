#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int hostnameToIP(char* hostname , char* ip)
{
    struct addrinfo* result;
    struct addrinfo hints;    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int error = getaddrinfo(hostname, NULL, &hints, &result);
    if(error != 0)
    {
        return -1;
    }
    error = getnameinfo(result->ai_addr, result->ai_addrlen, ip, 16, NULL, 0, NI_NUMERICHOST);
    return error;
}