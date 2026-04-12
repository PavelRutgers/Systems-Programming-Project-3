#include "shell.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int apply_redirection(const command_t *cmd) {
    int fd;

    if (cmd->input_file != NULL) {
        fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input_file);
            return -1;
        }
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->output_file != NULL) {
        fd = open(cmd->output_file,
                  O_WRONLY | O_CREAT | O_TRUNC,
                  0640);
        if (fd < 0) {
            perror(cmd->output_file);
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

int run_builtin_with_redirection(shell_t *shell, const command_t *cmd) {
    int saved_stdin = -1;
    int saved_stdout = -1;
    int result;

    if (cmd->input_file != NULL) {
        saved_stdin = dup(STDIN_FILENO);
        if (saved_stdin < 0) {
            perror("dup");
            return 1;
        }
    }

    if (cmd->output_file != NULL) {
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout < 0) {
            perror("dup");
            if (saved_stdin >= 0) {
                close(saved_stdin);
            }
            return 1;
        }
    }

    if (apply_redirection(cmd) != 0) {
        if (saved_stdin >= 0) {
            close(saved_stdin);
        }
        if (saved_stdout >= 0) {
            close(saved_stdout);
        }
        return 1;
    }

    result = run_builtin(shell, cmd);

    if (saved_stdin >= 0) {
        if (dup2(saved_stdin, STDIN_FILENO) < 0) {
            perror("dup2");
        }
        close(saved_stdin);
    }

    if (saved_stdout >= 0) {
        if (dup2(saved_stdout, STDOUT_FILENO) < 0) {
            perror("dup2");
        }
        close(saved_stdout);
    }

    return result;
}

int execute_job(shell_t *shell, const job_t *job) {
    pid_t pid;
    int status;
    char *path;

    if (job == NULL || job->count == 0) {
        return 0;
    }

    if (job->count != 1) {
        fprintf(stderr, "pipelines not implemented yet\n");
        return 1;
    }

    if (job->commands[0].argc == 0) {
        return 0;
    }

    if (is_builtin(job->commands[0].argv[0])) {
        return run_builtin_with_redirection(shell, &job->commands[0]);
    }

    path = resolve_command_path(job->commands[0].argv[0]);
    if (path == NULL) {
        fprintf(stderr, "%s: command not found\n", job->commands[0].argv[0]);
        return 1;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        free(path);
        return 1;
    }

    if (pid == 0) {
        if (apply_redirection(&job->commands[0]) != 0) {
            free(path);
            _exit(1);
        }

        execv(path, job->commands[0].argv);
        perror("execv");
        free(path);
        _exit(1);
    }

    free(path);

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return 1;
}