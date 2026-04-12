#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int is_builtin(const char *name) {
    if (name == NULL) {
        return 0;
    }

    return strcmp(name, "cd") == 0 ||
           strcmp(name, "pwd") == 0 ||
           strcmp(name, "which") == 0 ||
           strcmp(name, "exit") == 0;
}

int run_builtin(shell_t *shell, const command_t *cmd) {
    char cwd[PATH_MAX];
    char *path;

    if (cmd == NULL || cmd->argc == 0) {
        return 0;
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc > 2) {
            fprintf(stderr, "cd: too many arguments\n");
            return 1;
        }

        if (cmd->argc == 1) {
            if (shell->home == NULL) {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }
            if (chdir(shell->home) != 0) {
                perror("cd");
                return 1;
            }
        } else {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd");
                return 1;
            }
        }

        return 0;
    }

    if (strcmp(cmd->argv[0], "pwd") == 0) {
        if (cmd->argc != 1) {
            return 1;
        }

        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("pwd");
            return 1;
        }

        printf("%s\n", cwd);
        return 0;
    }

    if (strcmp(cmd->argv[0], "which") == 0) {
        if (cmd->argc != 2) {
            return 1;
        }

        if (is_builtin(cmd->argv[1])) {
            return 1;
        }

        path = resolve_command_path(cmd->argv[1]);
        if (path == NULL) {
            return 1;
        }

        printf("%s\n", path);
        free(path);
        return 0;
    }

    if (strcmp(cmd->argv[0], "exit") == 0) {
        return 0;
    }

    return 1;
}