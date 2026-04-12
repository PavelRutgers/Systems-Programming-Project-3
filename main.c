#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    shell_t shell;
    char line[INPUT_BUFSIZE];
    ssize_t nread;

    shell.should_exit = 0;
    shell.last_status = 0;
    shell.home = getenv("HOME");

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        shell.input_fd = open(argv[1], O_RDONLY);
        if (shell.input_fd < 0) {
            perror("open");
            return EXIT_FAILURE;
        }
        shell.interactive = 0;
    } else {
        shell.input_fd = STDIN_FILENO;
        shell.interactive = isatty(STDIN_FILENO);
    }

    if (shell.interactive) {
        print_welcome(&shell);
    }

    while (!shell.should_exit) {
        token_list_t tokens;
        job_t job;
        size_t i;

        if (shell.interactive) {
            print_prompt(&shell);
        }

        nread = shell_read_line(shell.input_fd, line, sizeof(line));
        if (nread < 0) {
            perror("read");
            break;
        }
        if (nread == 0) {
            break;
        }

        if (line[0] == '\0') {
            continue;
        }

        if (tokenize_line(line, &tokens) != 0) {
            fprintf(stderr, "tokenizer error\n");
            continue;
        }

        if (tokens.size == 0) {
            free_tokens(&tokens);
            continue;
        }

        if (parse_tokens(&tokens, &job) != 0) {
            fprintf(stderr, "parse error\n");
            free_tokens(&tokens);
            continue;
        }

        printf("command count = %zu\n", job.count);
    for (i = 0; i < job.commands[0].argc; i++) {
        printf("argv[%zu] = \"%s\"\n", i, job.commands[0].argv[i]);
    }
    if (job.commands[0].input_file != NULL) {
        printf("input_file = \"%s\"\n", job.commands[0].input_file);
    }
    if (job.commands[0].output_file != NULL) {
        printf("output_file = \"%s\"\n", job.commands[0].output_file);
    }

        if (job.count > 0 &&
            job.commands[0].argc > 0 &&
            strcmp(job.commands[0].argv[0], "exit") == 0) {
            shell.should_exit = 1;
        }

        free_job(&job);
        free_tokens(&tokens);
    }

    if (shell.interactive) {
        print_goodbye(&shell);
    }

    if (argc == 2) {
        close(shell.input_fd);
    }

    return EXIT_SUCCESS;
}