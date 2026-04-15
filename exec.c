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
        fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
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
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout >= 0) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }

    return result;
}

static int execute_pipeline(shell_t *shell, const job_t *job) {
    size_t n = job->count;
    size_t i, j;
    int *pipefds = NULL;
    pid_t *pids = NULL;
    int last_status = 1;

    (void)shell;

    if (n == 0) {
        return 0;
    }

    pipefds = malloc((n > 1 ? 2 * (n - 1) : 0) * sizeof(int));
    pids = malloc(n * sizeof(pid_t));
    if ((n > 1 && pipefds == NULL) || pids == NULL) {
        free(pipefds);
        free(pids);
        return 1;
    }

    for (i = 0; i + 1 < n; i++) {
        if (pipe(&pipefds[2 * i]) < 0) {
            perror("pipe");
            free(pipefds);
            free(pids);
            return 1;
        }
    }

    for (i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            free(pipefds);
            free(pids);
            return 1;
        }

        if (pids[i] == 0) {
            char *path;

            if (i > 0) {
                if (dup2(pipefds[2 * (i - 1)], STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            if (i + 1 < n) {
                if (dup2(pipefds[2 * i + 1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            for (j = 0; j + 1 < n; j++) {
                close(pipefds[2 * j]);
                close(pipefds[2 * j + 1]);
            }

            if (apply_redirection(&job->commands[i]) != 0) {
                _exit(1);
            }

            if (is_builtin(job->commands[i].argv[0])) {
                _exit(run_builtin(shell, &job->commands[i]));
            }

            path = resolve_command_path(job->commands[i].argv[0]);
            if (path == NULL) {
                fprintf(stderr, "%s: command not found\n", job->commands[i].argv[0]);
                _exit(1);
            }

            execv(path, job->commands[i].argv);
            perror("execv");
            free(path);
            _exit(1);
        }
    }

    for (i = 0; i + 1 < n; i++) {
        close(pipefds[2 * i]);
        close(pipefds[2 * i + 1]);
    }

    for (i = 0; i < n; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) < 0) {
            perror("waitpid");
            continue;
        }

        if (i == n - 1) {
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_status = 128 + WTERMSIG(status);
            }
        }
    }

    free(pipefds);
    free(pids);
    return last_status;
}

int execute_job(shell_t *shell, const job_t *job) {
    pid_t pid;
    int status;
    char *path;

    if (job == NULL || job->count == 0) {
        return 0;
    }

    if (job->count > 1) {
        return execute_pipeline(shell, job);
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