#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>

void str_split(char* str, const char* tok, char*** res, int* count) {

    // To avoid Segmentation fault we need to convert to char array....
    // Maybe I need to write a new strtok function?
    char buff[10001];
    strncpy(buff, str, 10001);

    // some var for split
    int _count = 0;
    char* _res[5000];
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

int is_match(const char* str, char* regex_str) {
    regex_t regex;
    int res;
    const int nmatch = 1;
    regmatch_t pmatch[nmatch];

    // compile regex
    if(regcomp(&regex, regex_str, REG_EXTENDED) != 0) {
        // if there contains an error
        regfree(&regex);
        return -1;
    }

    // check match
    res = regexec(&regex, str, nmatch, pmatch, 0);

    // free regex data
    regfree(&regex);

    // return result
    if(res == REG_NOMATCH){
        return 0;// no match
    } else {
        return 1;// match
    }

}
