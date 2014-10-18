#include <string.h>
#include "parser.h"
#include <unistd.h>
#include "util.h"
#include "token.h"
#include <stdlib.h>
#include <regex.h>


void _read_and_parse();
int _get_type(char* str);

int init_source_fd(int _sfd) {
    _source_fd = _sfd;
}

void _read_and_parse() {
    int _n_bytes = read(_source_fd, _p_buffer, 10001);

    if(_n_bytes <= 0) { // end of stream or error
        // TODO: need to handle error
        token_node_t* new_node = malloc(sizeof(token_node_t));
        new_node->token_str = NULL;
        new_node->type = FEOF;
        insert_node(&_token_list, new_node);
    }
    char** _result;
    int _num_of_token;
    str_split(_p_buffer, " ", &_result, &_num_of_token);
    int _c;
    for(_c=0; _c<_num_of_token; _c++) {
        token_node_t* new_node = malloc(sizeof(token_node_t));
        char* tmp_str = _result[_c];
        int type = _get_type(tmp_str);

        // set token string to node
        new_node->token_str = malloc(strlen(tmp_str));
        strcpy(new_node->token_str, tmp_str);
        free(tmp_str);

        // set type to node
        new_node->type = type;

        insert_node(&_token_list, new_node);
    }
}

int _get_type(char* str) {
    if(is_match(str, "\\|[[:digit:]]+")) {
        return PIPE_N;
    } else if(is_match(str, "\\|")) {
        return PIPE;
    } else if(is_match(str, ">")) {
        return PASS;
    } else if(is_match(str, "\n")) {
        return NEW_LINE;
    } else {
        return CMD;
    }
}

int _is_match(const char* str, char* regex_str) {
    regex_t regex;
    int res;
    const int nmatch = 1;
    regmatch_t pmatch[nmatch];

    // compile regex
    if(regcomp(&regex, regex_str, REG_EXTENDED) != 0) {
        // if there is an error, return fail
        return -1;
    }

    res = regexec(&regex, str, nmatch, pmatch, 0);
    if(res == REG_NOMATCH){
        return 0;
    } else {
        return 1;
    }
}

int next_token(char** token_string) {

    // if all tokens are processed
    // get next line
    if(_token_list == NULL) {
        _read_and_parse();
    }

    token_node_t* _token_node = pull_node(_token_list);

    // get token string
    char* tmp_str = _token_node->token_str;
    *token_string = malloc(sizeof(char)*strlen(tmp_str));
    strcpy(*token_string, tmp_str);

    // get token type
    int _type = _token_node->type;

    free(_token_node);

    return _type;
}
