#include <sys/wait.h>
#include "server.h"
#include "cmd_node.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "node_list.h"
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "tokenizer.h"


char* welcome_message = "****************************************\n** Welcome to the information server. **\n****************************************\n";
char* error_msg1 = "Unknown command: [";
char* error_msg2 = "].";

void serve(int client_fd) {

    // check if fd is right
    if(client_fd == -1) {
        fprintf(stderr, "Error! can't use file discription!");
        return;
    }

    // init tokenizer input source
    init_source_fd(client_fd);

    // replace stdin, stdout, stderr to client fd
    close(0);
    close(1);
    close(2);
    dup(client_fd);
    dup(client_fd);
    dup(client_fd);

    // print welcome message
    write(1, welcome_message, strlen(welcome_message));

    // write "% " to client
    write(1, "% ", 2);
    usleep(100);

    cmd_node_t* cmd_node_list = NULL;
    parse_tokens(&cmd_node_list);

    while(cmd_node_list != NULL) {
        // process commands

        // despatch node to right palce
        // first, check special command(setenv or printenv...)
        if(cmd_node_list->pipe_count == -1) {
            // only new line, pass
            cmd_node_list = cmd_node_list->next_node;
        } else if(strcmp(cmd_node_list->cmd, "printenv") == 0) {
            char* env_name = cmd_node_list->args[1];
            if(env_name != NULL) {
                char* env_val = getenv(env_name);
                write(1, env_name, strlen(env_name));
                write(1, "=", 2);
                write(1, env_val, strlen(env_val));
                write(1, "\n", 1);
                usleep(100);
            }
            cmd_node_list = cmd_node_list->next_node;
        } else if(strcmp(cmd_node_list->cmd, "setenv") == 0) {
            char* env_name = cmd_node_list->args[1];
            char* env_val = cmd_node_list->args[2];
            setenv(env_name, env_val, 1);
            cmd_node_list = cmd_node_list->next_node;
            usleep(100);
        } else if(strcmp(cmd_node_list->cmd, "exit") == 0) {
            // TODO: close all child process....
            close(client_fd);
            exit(0);
        } else {
            while(cmd_node_list != NULL) {
                if(cmd_node_list->pipe_count == -1) {
                    // only new line, pass and wait for next line
                    cmd_node_list = cmd_node_list->next_node;
                    break;
                } else {
                    cmd_node_t* node_to_place = pull_cmd_node(&cmd_node_list);
                    if(place_cmd_node(node_to_place) == -1) {
                        // command not found
                        write(1, error_msg1, strlen(error_msg1));
                        write(1, node_to_place->cmd, strlen(node_to_place->cmd));
                        write(1, error_msg2, strlen(error_msg2));
                        write(1, "\n", 1);
                        free_cmd_list(&cmd_node_list);
                    }
                }
            }
        }
        execute_cmd_node_list_chain();
        write(1, "% ", 2);
        parse_tokens(&cmd_node_list);
    }
}

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
    // check stdin
    // if last_pipefd[0] is stdin, do not close it
    if(last_pipefd[0] != 0){
        close(0);
        dup(last_pipefd[0]);
        close(last_pipefd[0]);
    }

    if(node->next_node != NULL){
        // create pipe
        int pipefd[2];
        pipe(pipefd);

        // fork
        int pid = fork();

        if(pid != 0) { // parent
            // close origin output and connect to pipe
            if(close(1) == -1){
                fprintf(stderr, "Can't close output");
            }
            if(dup(pipefd[1]) == -1) {
                fprintf(stderr, "Can't dup output from fd: %d", pipefd[1]);
            }
            if(close(pipefd[1]) == -1) {
                fprintf(stderr, "Can't close fd: %d", pipefd[1]);
            }

            // execute command with paramaters
            if(execvp(node->cmd, node->args) == -1) {
                fprintf(stderr, "Can't execute command: %s", node->cmd);
            }
        } else { // child

            if(close(pipefd[1]) == -1) {
                fprintf(stderr, "Can't close fd: %d", pipefd[1]);
            }
            exec_cmd_node(node->next_node, pipefd);
        }
    } else {
        // fork and execute it
        int pid = fork();
        if(pid == 0) {
            if(node->pipe_to_file == 1) {
                int file_fd = open(node->filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
                if(file_fd == -1) {
                    fprintf(stderr, "Can't open file %s\n", node->filename);
                }
                if(close(1) == -1) {
                    fprintf(stderr, "can't close output\n");
                } // close output
                if(dup(file_fd) == -1) {
                    fprintf(stderr, "can't dup\n");
                }// replace with file fd

                if(close(file_fd) == -1) {
                    fprintf(stderr, "can't close file fd\n");
                } // close file fd
            }

            // child to execute
            execvp(node->cmd, node->args);
        }
    }
}

int place_cmd_node(cmd_node_t* cmd_node) {
    node_t* tmp_node_chain = node_list_chain;

    // get cmd node and let list to point to next

    char* env_path = getenv("PATH");
    if(is_cmd_exist(cmd_node->cmd, env_path) == -1) {
        // command, executeable not found
        // return -1
        return -1;
    }

    int insert_tag = 0; // if cmd insert to chain, set to 1
    while(tmp_node_chain != NULL) {
        if(tmp_node_chain->tail->pipe_count == 1) {
            tmp_node_chain->tail->pipe_count--;
            cmd_node_t* node_to_insert = clone_cmd_node(cmd_node);
            insert_cmd_node(&(tmp_node_chain->tail), node_to_insert);
            tmp_node_chain->tail = cmd_node;
            insert_tag = 1;
        } else {
            tmp_node_chain->tail->pipe_count--;
        }
        tmp_node_chain = tmp_node_chain->next_node;
    }

    if(insert_tag == 0) {
        // if cmd not insert to any chain node, create a new node
        node_t* new_chain_node = malloc(sizeof(node_t));
        new_chain_node->head = (cmd_node_t*)cmd_node;
        new_chain_node->tail = (cmd_node_t*)cmd_node;
        new_chain_node->next_node = NULL;

        insert_to_node_chain(&node_list_chain, new_chain_node);
    } else {
        free_cmd_node(cmd_node);
    }
    return 0;
}

void execute_cmd_node_list_chain() {
    node_t* tmp_node_chain = node_list_chain;
    while(tmp_node_chain != NULL) {
        if(tmp_node_chain->tail->pipe_count == 0) {
            // execute!
            cmd_node_t* _to_exec = tmp_node_chain->head;

            remove_from_chain(tmp_node_chain);
            int init_fd[2] = {0, 1};
            exec_cmd_node(_to_exec, init_fd);
            int st;
            wait(&st);
        }
        tmp_node_chain = tmp_node_chain->next_node;
    }
}


void remove_from_chain(node_t* node_to_remove) {
    if(node_to_remove == NULL) {
        return;
    }

    if(node_to_remove == node_list_chain) {
        node_list_chain = node_list_chain->next_node;
    } else {
        node_t* tmp_node_chain = node_list_chain;
        while(tmp_node_chain->next_node != node_to_remove) {
            tmp_node_chain = tmp_node_chain->next_node;
        }
        if(tmp_node_chain != NULL) {
            tmp_node_chain->next_node = node_to_remove->next_node;
        }
    }
    node_to_remove->next_node = NULL;
    node_to_remove->head = NULL;
    node_to_remove->tail = NULL;
    free(node_to_remove);

}
