#include "shell.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

void print_welcome(shell_t *shell) {
    (void)shell;
    printf("Welcome to my shell!\n");
}

void print_goodbye(shell_t *shell) {
    (void)shell;
    printf("Exiting my shell.\n");
}

void print_prompt(shell_t *shell) {
    char cwd[PATH_MAX];
    char display[PATH_MAX];
    size_t home_len;

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("$ ");
        fflush(stdout);
        return;
    }

    if (shell->home != NULL) {
        home_len = strlen(shell->home);
        if (strncmp(cwd, shell->home, home_len) == 0) {
            if (cwd[home_len] == '\0') {
                snprintf(display, sizeof(display), "~");
            } else if (cwd[home_len] == '/') {
                snprintf(display, sizeof(display), "~%s", cwd + home_len);
            } else {
                snprintf(display, sizeof(display), "%s", cwd);
            }
        } else {
            snprintf(display, sizeof(display), "%s", cwd);
        }
    } else {
        snprintf(display, sizeof(display), "%s", cwd);
    }

    printf("%s$ ", display);
    fflush(stdout);
}