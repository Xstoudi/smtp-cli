#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

bool isValidIP(char* ipAddress)
{
    int blockCount = 0;
    char* token = strtok(ipAddress, ".");
    while(token != NULL)
    {
        blockCount++;
        char* endPointer;
        int block = strtol(token, &endPointer, 10);
        if(endPointer == token || block < 0 || block > 255)
        {
            return false;
        }

        token = strtok(NULL, ".");
    }
    if(blockCount != 4)
    {
        return false;
    }
    return true;
}

// REF: https://stackoverflow.com/a/122721
char* trim(char *str)
{
    char* end;

    // Trim leading space
    while(isspace((unsigned char)*str))
    {
        str++;
    }

    if(*str == 0)
    {
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    // Write new null terminator character
    end[1] = '\0';

    return str;
}