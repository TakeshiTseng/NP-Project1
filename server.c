#include "server.h"
#include "cmd_node.h"
#include <unistd.h>
#include <string.h>
#include "util.h"
#include <sys/stat.h>
#include "node_list.h"
#include <stdlib.h>
#include <stdio.h>

int is_cmd_exist(char* cmd, char* env_path) {
    // first, we need to parse env_path
    // split by ":"
    int p_num; // number of paths
    char** path_array; // array of paths

    str_split(env_path, ":", &path_array, &p_num);
    // second, search this file from these path
    // by using "stat()" function to check

    int c;
    struct stat sb;

    for(c=0; c<p_num; c++) {
        char* path = (char*)malloc(sizeof(char)*512);
        sprintf(path, "%s/%s", path_array[c], cmd);

        if(stat(path, &sb) != -1) {
            // if result is not -1, return 1
            return 1;
        }
    }
    return -1;
    // if all path are not found, return -1
}

void exec_cmd_node(cmd_node_t* node, int last_pipefd[2]) {
    close(0);
    dup(last_pipefd[0]);
    close(last_pipefd[0]);

    if(node->next_node != NULL){
        // create pipe
        int pipefd[2];
        pipe(pipefd);

        // fork
        int pid = fork();

        if(pid != 0) { // parent
            // close origin output and connect to pipe
            close(1);
            dup(pipefd[1]);
            close(pipefd[1]);
            execvp(node->cmd, node->args);
        } else { // child
            close(pipefd[1]);
            exec_cmd_node(node->next_node, pipefd);
        }
    } else {
        execvp(node->cmd, node->args);
    }
}

void exec_cmd_node_list(cmd_node_t* node){
    int pipefd[2] = {0, 1};
    exec_cmd_node(node, pipefd);
}
