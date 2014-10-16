#ifndef __SERVER_H__
#define __SERVER_H__

#include "cmd_node.h"
#include "node_list.h"

void exec_cmd_node_list(cmd_node_t* head);
void place_cmd_node(node_list_t* head);

void serve(int client_fd);


#endif
