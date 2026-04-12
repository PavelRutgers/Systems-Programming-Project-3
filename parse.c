#include "shell.h"

#include <stdlib.h>
#include <string.h>

void free_job(job_t *job) {
    size_t i;
    size_t j;

    for (i = 0; i < job->count; i++) {
        for (j = 0; j < job->commands[i].argc; j++) {
            free(job->commands[i].argv[j]);
        }
        free(job->commands[i].argv);

        if (job->commands[i].input_file != NULL) {
            free(job->commands[i].input_file);
        }
        if (job->commands[i].output_file != NULL) {
            free(job->commands[i].output_file);
        }
    }

    free(job->commands);
    job->commands = NULL;
    job->count = 0;
}

int parse_tokens(const token_list_t *tokens, job_t *job) {
    size_t i;
    size_t argc = 0;
    command_t *cmd;

    job->commands = NULL;
    job->count = 0;

    if (tokens->size == 0) {
        return 0;
    }

    job->commands = malloc(sizeof(command_t));
    if (job->commands == NULL) {
        return -1;
    }

    job->count = 1;
    cmd = &job->commands[0];

    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->argv = malloc((tokens->size + 1) * sizeof(char *));
    if (cmd->argv == NULL) {
        free(job->commands);
        job->commands = NULL;
        job->count = 0;
        return -1;
    }

    for (i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->items[i], "<") == 0) {
            if (i + 1 >= tokens->size) {
                free_job(job);
                return -1;
            }
            cmd->input_file = malloc(strlen(tokens->items[i + 1]) + 1);
            if (cmd->input_file == NULL) {
                free_job(job);
                return -1;
            }
            strcpy(cmd->input_file, tokens->items[i + 1]);
            i++;
        } else if (strcmp(tokens->items[i], ">") == 0) {
            if (i + 1 >= tokens->size) {
                free_job(job);
                return -1;
            }
            cmd->output_file = malloc(strlen(tokens->items[i + 1]) + 1);
            if (cmd->output_file == NULL) {
                free_job(job);
                return -1;
            }
            strcpy(cmd->output_file, tokens->items[i + 1]);
            i++;
        } else {
            cmd->argv[argc] = malloc(strlen(tokens->items[i]) + 1);
            if (cmd->argv[argc] == NULL) {
                free_job(job);
                return -1;
            }
            strcpy(cmd->argv[argc], tokens->items[i]);
            argc++;
        }
    }

    cmd->argv[argc] = NULL;
    cmd->argc = argc;

    return 0;
}