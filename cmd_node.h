#ifndef __CMD_NODE_H__
#define __CMD_NODE_H__

struct cmd_node {
	char* cmd;
	char** args;
	int pipe_count;
	struct cmd_node* next_node;
};

typedef struct cmd_node cmd_node_t;

#endif
