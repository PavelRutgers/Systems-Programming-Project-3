#include "shell.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *resolve_command_path(const char *name) {
    const char *dirs[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    size_t i;
    size_t len;
    char *path;

    if (name == NULL) {
        return NULL;
    }

    if (strchr(name, '/') != NULL) {
        if (access(name, X_OK) == 0) {
            path = malloc(strlen(name) + 1);
            if (path == NULL) {
                return NULL;
            }
            strcpy(path, name);
            return path;
        }
        return NULL;
    }

    for (i = 0; i < 3; i++) {
        len = strlen(dirs[i]) + 1 + strlen(name) + 1;
        path = malloc(len);
        if (path == NULL) {
            return NULL;
        }

        snprintf(path, len, "%s/%s", dirs[i], name);

        if (access(path, X_OK) == 0) {
            return path;
        }

        free(path);
    }

    return NULL;
}