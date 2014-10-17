#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

#include "cmd_node.h"

struct node {
    cmd_node_t* head;
    cmd_node_t* tail;
    struct node* next;
};

typedef struct node node_t;


#endif
