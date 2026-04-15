#include "shell.h"

#include <stdlib.h>
#include <string.h>

static int is_special_token(const char *s) {
    return strcmp(s, "<") == 0 ||
           strcmp(s, ">") == 0 ||
           strcmp(s, "|") == 0;
}

static void init_command(command_t *cmd) {
    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
}

static void free_command(command_t *cmd) {
    size_t i;

    if (cmd->argv != NULL) {
        for (i = 0; i < cmd->argc; i++) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
    }

    free(cmd->input_file);
    free(cmd->output_file);

    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
}

void free_job(job_t *job) {
    size_t i;

    for (i = 0; i < job->count; i++) {
        free_command(&job->commands[i]);
    }

    free(job->commands);
    job->commands = NULL;
    job->count = 0;
}

static int add_arg(command_t *cmd, const char *text) {
    char **new_argv;
    char *copy;

    new_argv = realloc(cmd->argv, (cmd->argc + 2) * sizeof(char *));
    if (new_argv == NULL) {
        return -1;
    }
    cmd->argv = new_argv;

    copy = malloc(strlen(text) + 1);
    if (copy == NULL) {
        return -1;
    }
    strcpy(copy, text);

    cmd->argv[cmd->argc] = copy;
    cmd->argc++;
    cmd->argv[cmd->argc] = NULL;
    return 0;
}

static int set_redirection(char **target, const char *filename) {
    char *copy;

    if (*target != NULL) {
        return -1;
    }

    copy = malloc(strlen(filename) + 1);
    if (copy == NULL) {
        return -1;
    }
    strcpy(copy, filename);
    *target = copy;
    return 0;
}

int parse_tokens(const token_list_t *tokens, job_t *job) {
    size_t i;
    size_t cmd_index;
    size_t pipe_count = 0;

    job->commands = NULL;
    job->count = 0;

    if (tokens->size == 0) {
        return 0;
    }

    for (i = 0; i < tokens->size; i++) {
        if (strcmp(tokens->items[i], "|") == 0) {
            pipe_count++;
        }
    }

    job->count = pipe_count + 1;
    job->commands = malloc(job->count * sizeof(command_t));
    if (job->commands == NULL) {
        job->count = 0;
        return -1;
    }

    for (i = 0; i < job->count; i++) {
        init_command(&job->commands[i]);
    }

    cmd_index = 0;

    for (i = 0; i < tokens->size; i++) {
        command_t *cmd = &job->commands[cmd_index];

        if (strcmp(tokens->items[i], "|") == 0) {
            if (cmd->argc == 0) {
                free_job(job);
                return -1;
            }
            cmd_index++;
            if (cmd_index >= job->count) {
                free_job(job);
                return -1;
            }
            continue;
        }

        if (strcmp(tokens->items[i], "<") == 0) {
            if (i + 1 >= tokens->size || is_special_token(tokens->items[i + 1])) {
                free_job(job);
                return -1;
            }
            if (set_redirection(&cmd->input_file, tokens->items[i + 1]) != 0) {
                free_job(job);
                return -1;
            }
            i++;
            continue;
        }

        if (strcmp(tokens->items[i], ">") == 0) {
            if (i + 1 >= tokens->size || is_special_token(tokens->items[i + 1])) {
                free_job(job);
                return -1;
            }
            if (set_redirection(&cmd->output_file, tokens->items[i + 1]) != 0) {
                free_job(job);
                return -1;
            }
            i++;
            continue;
        }

        if (add_arg(cmd, tokens->items[i]) != 0) {
            free_job(job);
            return -1;
        }
    }

    for (i = 0; i < job->count; i++) {
        if (job->commands[i].argc == 0) {
            free_job(job);
            return -1;
        }
    }

    return 0;
}