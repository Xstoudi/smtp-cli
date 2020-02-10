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