#ifndef __SERVER_H__
#define __SERVER_H__

#include "cmd_node.h"
#include "node_list.h"


int place_cmd_node(cmd_node_t* cmd_node);
int is_cmd_exist(char* cmd, char* env_path);
void exec_cmd_node(cmd_node_t* node, int last_pipefd[2]);
void execute_cmd_node_list_chain();
void serve(int client_fd);
void remove_from_chain(node_t* node_to_remove);
node_t* node_list_chain;

#endif
