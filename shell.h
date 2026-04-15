#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>
#include <sys/types.h>

#define INPUT_BUFSIZE 4096

typedef struct {
    int interactive;
    int should_exit;
    int last_status;
    int input_fd;
    char *home;
} shell_t;
typedef struct {
    char **items;
    size_t size;
    size_t capacity;
} token_list_t;
typedef struct {
    char **argv;
    size_t argc;
    char *input_file;
    char *output_file;
} command_t;

typedef struct {
    command_t *commands;
    size_t count;
} job_t;


/* input.c */
ssize_t shell_read_line(int fd, char *buffer, size_t max_len);

/* util.c */
void print_prompt(shell_t *shell);
void print_welcome(shell_t *shell);
void print_goodbye(shell_t *shell);

int tokenize_line(const char *line, token_list_t *tokens);
void free_tokens(token_list_t *tokens);

int parse_tokens(const token_list_t *tokens, job_t *job);
void free_job(job_t *job);
char *resolve_command_path(const char *name);

int is_builtin(const char *name);
int run_builtin(shell_t *shell, const command_t *cmd);

int apply_redirection(const command_t *cmd);
int run_builtin_with_redirection(shell_t *shell, const command_t *cmd);
int execute_job(shell_t *shell, const job_t *job);
int expand_job(job_t *job);

#endif