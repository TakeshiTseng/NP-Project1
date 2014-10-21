#ifndef __CMD_NODE_H__
#define __CMD_NODE_H__

struct cmd_node {
    char* cmd;
    char** args;
    int pipe_to_file;
    char* filename;
    int pipe_count;
    struct cmd_node* next_node;
};

typedef struct cmd_node cmd_node_t;


void insert_cmd_node(cmd_node_t** list, cmd_node_t* node);
cmd_node_t* pull_cmd_node(cmd_node_t** list);

#endif
