#include "cmd_node.h"
#include <stdio.h>
#include <stdlib.h>

void insert_cmd_node(cmd_node_t** head, cmd_node_t* node) {

    if(*head == NULL) {
        *head = node;
        (*head)->next_node = NULL;
    } else {
        cmd_node_t* _p = *head;
        while(_p->next_node != NULL) {
            _p = _p->next_node;
        }
        _p->next_node = node;
        node->next_node = NULL;
    }
}

cmd_node_t* pull_cmd_node(cmd_node_t** list) {
    if(*list == NULL) {
        return NULL;
    } else {
        cmd_node_t* _to_return = *list;
        *list = (*list)->next_node;
        _to_return->next_node = NULL;
        return _to_return;
    }
}

void free_cmd_list(cmd_node_t** cmd_list) {
    while(*cmd_list != NULL) {
        cmd_node_t* tmp = *cmd_list;
        (*cmd_list) = (*cmd_list)->next_node;
        tmp->next_node = NULL;
        free(tmp);
    }
}
