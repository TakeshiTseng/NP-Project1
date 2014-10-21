#ifndef __PARSER_H__
#define __PARSER_H__
#include "node_list.h"
#include "token.h"

// states

#define STATE_INIT 0
#define STATE_CMD 1
#define STATE_FILE 2
#define STATE_ARGS 3
#define STATE_NW 4

void parse(token_node_t* token_list, cmd_node_t** node_list);



#endif
