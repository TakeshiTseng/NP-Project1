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
#include "pipe_node.h"

char* welcome_message = "****************************************\n** Welcome to the information server. **\n****************************************\n";
char* error_msg1 = "Unknown command: [";
char* error_msg2 = "].";

void serve(int client_fd) {

    cmd_exec_list = NULL;
    pipe_list = NULL;

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
    int state;

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
                printf("%s=%s\n", env_name, env_val);
                fflush(stdout);
            }
            cmd_node_list = cmd_node_list->next_node;
        } else if(strcmp(cmd_node_list->cmd, "setenv") == 0) {
            char* env_name = cmd_node_list->args[1];
            char* env_val = cmd_node_list->args[2];
            setenv(env_name, env_val, 1);
            cmd_node_list = cmd_node_list->next_node;
        } else if(strcmp(cmd_node_list->cmd, "exit") == 0) {
            // TODO: close all child process....
            return;
        } else {
            while(cmd_node_list != NULL) {
                if(cmd_node_list->pipe_count == -1) {
                    // new line, execute
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

int place_cmd_node(cmd_node_t* cmd_node) {

    char* env_path = getenv("PATH");
    if(is_cmd_exist(cmd_node->cmd, env_path) == -1) {
        // command, executeable not found
        // return -1
        return -1;
    }

    int pipe_count = cmd_node->pipe_count;
    int pid = -1;
    int input_pipe_fd = -1;
    int output_pipe_fd = -1;
    decrease_all_pipe_node(pipe_list);

    // get this process input source
    pipe_node_t* in_pipe_node = find_pipe_node_by_count(pipe_list, 0);
    if(in_pipe_node != NULL) {
        input_pipe_fd = in_pipe_node->in_fd;
        close(in_pipe_node->out_fd);
        in_pipe_node->count--;
    }

    // get this process output source
    pipe_node_t* out_pipe_node = find_pipe_node_by_count(pipe_list, pipe_count);
    if(out_pipe_node != NULL) {
        output_pipe_fd = out_pipe_node->out_fd;
    } else if(cmd_node->pipe_to_file == 1){
        output_pipe_fd = get_file_fd(cmd_node->filename);
    } else if(cmd_node->pipe_count != 0) {
        int new_pipe_fd[2];
        pipe(new_pipe_fd);
        out_pipe_node = malloc(sizeof(pipe_node_t));
        out_pipe_node->count = cmd_node->pipe_count;
        out_pipe_node->in_fd = new_pipe_fd[0];
        out_pipe_node->out_fd = new_pipe_fd[1];
        out_pipe_node->next_node = NULL;
        insert_pipe_node(&pipe_list, out_pipe_node);

        output_pipe_fd = new_pipe_fd[1];
    }


    pid = fork();
    if(pid == 0) {
        if(input_pipe_fd != -1) {
            // not use stdin
            close(0);
            dup(input_pipe_fd);
            close(input_pipe_fd);
        }

        // out
        if(out_pipe_node != NULL) {
            close(1);
            dup(out_pipe_node->out_fd);
            close(out_pipe_node->out_fd);
        } else if(cmd_node->pipe_to_file == 1) {
            close(1);
            dup(output_pipe_fd);
            close(output_pipe_fd);
        }

        execvp(cmd_node->cmd, cmd_node->args);
    } else if(pipe_count != 0) {
        int st;
        waitpid(pid, &st, 0);
        if(input_pipe_fd != -1) {
            close(input_pipe_fd);
        }
    } else {
        int st;
        waitpid(pid, &st, 0);
        close_unused_fd();
    }
    return 0;
}

int get_file_fd(char* filename) {
    return open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH);
}

void close_unused_fd() {
    pipe_node_t* new_list = NULL;
    while(pipe_list != NULL) {
        pipe_node_t* tmp_node = pipe_list;
        pipe_list = pipe_list->next_node;
        tmp_node->next_node = NULL;

        if(tmp_node->count <= 0) {
            if(fcntl(tmp_node->out_fd, F_GETFD) != -1) {
                close(tmp_node->out_fd);
            }
            if(fcntl(tmp_node->in_fd, F_GETFD) != -1) {
                close(tmp_node->in_fd);
            }
        } else {
            insert_pipe_node(&new_list, tmp_node);
        }
    }

    pipe_list = new_list;
}


