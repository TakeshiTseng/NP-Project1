#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tokenizer.h"
#include "cmd_node.h"

int _next_state(int state, int next_token_type);
void _insert_arg(arg_node_t** arg_list, arg_node_t* node);


cmd_node_t* parse_tokens() {

    int state = STATE_INIT;
    int pre_state = STATE_INIT;
    cmd_node_t* current_cmd_node = NULL;
    cmd_node_t* cmd_node_list = NULL;

    // token
    int token_type;
    char* token_str;

    token_type = next_token(&token_str);
    while(token_type != NEW_LINE) {
        if(state == STATE_INIT){
            if(current_cmd_node == NULL) {
                state = _next_state(state, token_type);
            } else {
                // pipe
                if(token_type == PIPE) {
                    current_cmd_node->pipe_count = 1;
                } else {
                    // pipt n
                    // cut first letter
                    char* pipe_n_str;
                    pipe_n_str = malloc(sizeof(token_str));
                    strcpy(pipe_n_str, (token_str+1));
                    // set number to pipe count
                    current_cmd_node->pipe_count = atoi(pipe_n_str);
                    free(pipe_n_str);
                }

                insert_cmd_node(&cmd_node_list, current_cmd_node);
                current_cmd_node = NULL;

                // get next token
                token_type = next_token(&token_str);
                // change to next state
                pre_state = state;
                state = _next_state(state, token_type);
            }
        } else if(state == STATE_CMD) {
            // from init state
            current_cmd_node = malloc(sizeof(cmd_node_t));

            // init data
            current_cmd_node->pipe_to_file = 0;// not to pipe to file
            current_cmd_node->pipe_count = 0;
            current_cmd_node->next_node = NULL;

            // copy text to cmd node
            current_cmd_node->cmd = malloc(sizeof(token_str));
            strcpy(current_cmd_node->cmd, token_str);

            // change to next node
            token_type = next_token(&token_str);

            // change state
            pre_state = state;
            state = _next_state(state, token_type);

        } else if(state == STATE_ARGS) {
            // from cmd
            // calculate how many arguments
            arg_node_t* arg_list = NULL;
            int count = 0;
            while(token_type == CMD) {
                arg_node_t* arg = malloc(sizeof(arg_node_t));
                arg->arg_str = malloc(sizeof(token_str));
                _insert_arg(&arg_list, arg);
                token_type = next_token(&token_str);
                count++;
            }

            current_cmd_node->args = malloc(sizeof(char*) * count);

            count = 0;

            while(arg_list != NULL) {
                current_cmd_node->args[count] = malloc(sizeof(arg_list->arg_str));
                strcpy(current_cmd_node->args[count], arg_list->arg_str);
                count++;

                // release unused node
                arg_node_t* node_to_release = arg_list;
                arg_list = arg_list->next_node;
                node_to_release->next_node = NULL;
                free(node_to_release);
            }

            pre_state = state;
            state = _next_state(state, token_type);
        } else if(state == STATE_FILE) {
            // release ">" token node
            token_type = next_token(&token_str);
            current_cmd_node->pipe_to_file = 1;
            current_cmd_node->filename = malloc(sizeof(token_str));
            strcpy(current_cmd_node->filename, token_str);


            token_type = next_token(&token_str);
            pre_state = state;
            state = _next_state(state, token_type);
        }
    }
    // new line
    if(pre_state == STATE_INIT) {
        // do nothing?
    } else if(pre_state == STATE_FILE || pre_state == STATE_ARGS || pre_state == STATE_CMD) {
        insert_cmd_node(&cmd_node_list, current_cmd_node);
    }
    return cmd_node_list;
}



int _next_state(int state, int next_token_type) {

    if(state == STATE_INIT) {
        if(next_token_type == CMD) {
            return STATE_CMD;
        } else if(next_token_type == NEW_LINE) {
            return STATE_NW;
        }
    } else if(state == STATE_CMD) {
        if(next_token_type == CMD) {
            return STATE_ARGS;
        } else if(next_token_type == PASS) {
            return STATE_FILE;
        } else if(next_token_type == NEW_LINE) {
            return STATE_NW;
        } else if(next_token_type == PIPE || next_token_type == PIPE_N) {
            return STATE_INIT;
        }
    } else if(state == STATE_ARGS) {
        if(next_token_type == CMD) {
            return STATE_ARGS;
        } else if(next_token_type == PIPE || next_token_type == PIPE_N) {
            return STATE_INIT;
        } else if(next_token_type == PASS) {
            return STATE_FILE;
        } else if(next_token_type == NEW_LINE) {
            return STATE_NW;
        }
    } else if(state == STATE_FILE) {
        if(next_token_type == PIPE || next_token_type == PIPE_N) {
            return STATE_INIT;
        } else if(next_token_type == NEW_LINE) {
            return STATE_NW;
        }
    }
    return -1;
}


void _insert_arg(arg_node_t** arg_list, arg_node_t* new_node) {
    if(*arg_list == NULL) {
        *arg_list = new_node;
    } else {
        arg_node_t* tmp_node = *arg_list;
        while(tmp_node->next_node != NULL) {
            tmp_node = tmp_node->next_node;
        }
        tmp_node->next_node = new_node;
        new_node->next_node = NULL;
    }
}
