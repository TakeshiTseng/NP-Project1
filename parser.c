#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int _next_state(int state, token_node_t* token_list);

void parse(token_node_t* token_list, cmd_node_t** node_list) {

    int state = STATE_INIT;
    int pre_state = STATE_INIT;
    cmd_node_t* current_cmd_node = NULL;

    while(token_list != NULL) {
        if(state == STATE_INIT){
            if(current_cmd_node == NULL) {
                state = _next_state(state, token_list);
            } else {
                // pipe
                token_node_t* pipe_node = token_list;
                if(pipe_node->type == PIPE) {
                    current_cmd_node->pipe_count = 1;
                } else {
                    // cut first letter
                    char* pipe_n_str;
                    pipe_n_str = malloc(sizeof(pipe_node->token_str));
                    strcpy(pipe_n_str, (pipe_node->token_str+1));
                    int pipe_n = atoi(pipe_n_str);
                    current_cmd_node->pipe_count = pipe_n;
                }

                token_node_t* tmp_node = token_list;
                token_list = token_list->next_node;

                // release unused node
                tmp_node->next_node = NULL;
                free(tmp_node);

                insert_cmd_node(node_list, current_cmd_node);
                current_cmd_node = NULL;

                // change to next state
                pre_state = state;
                state = _next_state(state, token_list);
            }
        } else if(state == STATE_CMD) {
            // from init state
            current_cmd_node = malloc(sizeof(cmd_node_t));

            // init data
            current_cmd_node->pipe_to_file = 0;// not to pipe to file
            current_cmd_node->pipe_count = 0;
            current_cmd_node->next_node = NULL;

            // copy text to cmd node
            current_cmd_node->cmd = malloc(sizeof(char) * (strlen(token_list->token_str) + 1));
            strcpy(current_cmd_node->cmd, token_list->token_str);


            // change to next node
            token_node_t* tmp_token_node = token_list;
            token_list = token_list->next_node;

            // change state
            pre_state = state;
            state = _next_state(state, token_list);

            // release memory
            tmp_token_node->next_node = NULL;
            free(tmp_token_node);
        } else if(state == STATE_ARGS) {
            // from cmd
            // calculate how many arguments
            token_node_t* tmp_node = token_list;
            int count = 0;
            while(tmp_node != NULL && tmp_node->type == CMD) {
                count++;
                tmp_node = tmp_node->next_node;
            }
            current_cmd_node->args = malloc(sizeof(char*) * count);

            count = 0;
            while(token_list != NULL && token_list->type == CMD) {
                // copy string to args
                current_cmd_node->args[count] = malloc(sizeof(char) * (strlen(token_list->token_str) + 1));
                strcpy(current_cmd_node->args[count], token_list->token_str);

                // change to next node
                token_node_t* tmp_token_node = token_list;
                token_list = token_list->next_node;

                // release previous node
                tmp_token_node->next_node = NULL;
                free(tmp_token_node);
            }

            pre_state = state;
            state = _next_state(state, token_list);
        } else if(state == STATE_FILE) {

            // release ">" token node
            token_node_t* tmp_node = token_list;
            token_list = token_list->next_node;
            tmp_node->next_node = NULL;
            free(tmp_node);

            current_cmd_node->filename = malloc(sizeof(char)*(strlen(token_list->token_str) + 1));
            strcpy(current_cmd_node->filename, token_list->token_str);

            tmp_node = token_list;
            token_list = token_list->next_node;

            // release unused node
            tmp_node->next_node = NULL;
            free(tmp_node);

            pre_state = state;
            state = _next_state(state, token_list);
        }
    }
}



int _next_state(int state, token_node_t* token_list) {
    int type = token_list->type;

    if(state == STATE_INIT) {
        if(type == CMD) {
            return STATE_CMD;
        } else if(type == NEW_LINE) {
            return STATE_NW;
        }
    } else if(state == STATE_CMD) {
        if(type == CMD) {
            return STATE_ARGS;
        } else if(type == PASS) {
            return STATE_FILE;
        } else if(type == NEW_LINE) {
            return STATE_NW;
        } else if(type == PIPE || type == PIPE_N) {
            return STATE_INIT;
        }
    } else if(state == STATE_ARGS) {
        if(type == CMD) {
            return STATE_ARGS;
        } else if(type == PIPE || type == PIPE_N) {
            return STATE_INIT;
        } else if(type == PASS) {
            return STATE_FILE;
        } else if(type == NEW_LINE) {
            return STATE_NW;
        }
    } else if(state == STATE_FILE) {
        if(type == PIPE || type == PIPE_N) {
            return STATE_INIT;
        } else if(type == NEW_LINE) {
            return STATE_NW;
        }
    }
    return -1;
}
