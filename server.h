#ifndef __SERVER_H__
#define __SERVER_H__

#include "cmd_node.h"
#include "node_list.h"

void place_cmd_node(node_t* node_list);
int is_cmd_exist(char* cmd, char* env_path);
void exec_cmd_node(cmd_node_t* node, int last_pipefd[2]);

void serve(int client_fd);


#endif
