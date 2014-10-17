#include "util.h"
#include <stdlib.h>
#include <string.h>

void str_split(char* str, const char* tok, char*** res, int* count) {

    // To avoid Segmentation fault we need to convert to char array....
    // Maybe I need to write a new strtok function?
    char buff[256];
    strncpy(buff, str, 256);

    // some var for split
    int _count = 0;
    char* _res[128];
    char* tmp_str;

    tmp_str = strtok(buff, tok);

    while(tmp_str != NULL) {
        int _len = strlen(tmp_str);
        _res[_count] = malloc(sizeof(char)*_len);
        strcpy(_res[_count], tmp_str);
        _count++;
        tmp_str = strtok(NULL, tok);
    }

    *count = _count;
    *res = _res;

}
