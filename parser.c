#include <string.h>
#include "parser.h"
#include <unistd.h>
#include "util.h"
#include "token.h"
#include <stdlib.h>


void _read_and_parse();

int init_source_fd(int _sfd) {
    _source_fd = _sfd;
}

void _read_and_parse() {
    read(_source_fd, _p_buffer, 10001);
    char** _result;
    int _num_of_token;
    str_split(_p_buffer, " ", &_result, &_num_of_token);
    int _c;
    for(_c=0; _c<_num_of_token; _c++) {
        token_node_t* new_node = malloc(sizeof(token_node_t));
        insert_node(&_token_list, new_node);
    }
}

int next_token(char** token_string) {

    // if all tokens are processed
    // get next line
    if(_token_list == NULL) {
        _read_and_parse();
    }

    token_node_t* _token_node = pull_node(_token_list);
    char* tmp_str = _token_node->token_str;
    *token_string = malloc(sizeof(char)*strlen(tmp_str));
    strcpy(*token_string, tmp_str);
    int _type = _token_node->type;

    free(_token_node);

    return _type;
}
