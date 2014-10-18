#include "token.h"
#include <stdio.h>

void insert_node(token_node_t** head, token_node_t* node) {

    if(*head == NULL) {
        *head = node;
    } else {
        token_node_t* _p = *head;
        // walk to last node
        while(_p->next_node != NULL) {
			_p = _p->next_node;
		}
        _p->next_node = node;
        node->next_node = NULL;
    }

}

token_node_t* pull_node(token_node_t** list) {
    if(*list == NULL) {
        return NULL;
    } else {
        token_node_t* _to_return = *list;
        *list = (*list)->next_node;
        return _to_return;
    }
}
