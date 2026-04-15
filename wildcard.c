#include "shell.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int has_wildcard(const char *s) {
    return strchr(s, '*') != NULL;
}

static int match_pattern(const char *pattern, const char *name) {
    const char *star;
    size_t prefix_len, suffix_len, name_len;

    star = strchr(pattern, '*');
    if (star == NULL) {
        return strcmp(pattern, name) == 0;
    }

    prefix_len = (size_t)(star - pattern);
    suffix_len = strlen(star + 1);
    name_len = strlen(name);

    if (prefix_len > 0 && strncmp(pattern, name, prefix_len) != 0) {
        return 0;
    }

    if (suffix_len > 0) {
        if (name_len < suffix_len) {
            return 0;
        }
        if (strcmp(name + name_len - suffix_len, star + 1) != 0) {
            return 0;
        }
    }

    if (name_len < prefix_len + suffix_len) {
        return 0;
    }

    return 1;
}

static int add_string(char ***array, size_t *count, const char *text) {
    char **new_array;
    char *copy;

    new_array = realloc(*array, (*count + 1) * sizeof(char *));
    if (new_array == NULL) {
        return -1;
    }
    *array = new_array;

    copy = malloc(strlen(text) + 1);
    if (copy == NULL) {
        return -1;
    }
    strcpy(copy, text);

    (*array)[*count] = copy;
    (*count)++;
    return 0;
}

static void free_string_array(char **array, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        free(array[i]);
    }
    free(array);
}

static int expand_token(const char *pattern, char ***results, size_t *result_count) {
    DIR *dir;
    struct dirent *entry;
    size_t count = 0;
    char **matches = NULL;
    int pattern_starts_hidden;

    *results = NULL;
    *result_count = 0;

    dir = opendir(".");
    if (dir == NULL) {
        return -1;
    }

    pattern_starts_hidden = (pattern[0] == '.');

    while ((entry = readdir(dir)) != NULL) {
        if (!pattern_starts_hidden && entry->d_name[0] == '.') {
            continue;
        }

        if (match_pattern(pattern, entry->d_name)) {
            if (add_string(&matches, &count, entry->d_name) != 0) {
                closedir(dir);
                free_string_array(matches, count);
                return -1;
            }
        }
    }

    closedir(dir);

    if (count == 0) {
        if (add_string(&matches, &count, pattern) != 0) {
            free_string_array(matches, count);
            return -1;
        }
    }

    *results = matches;
    *result_count = count;
    return 0;
}

static int expand_command(command_t *cmd) {
    char **new_argv = NULL;
    size_t new_argc = 0;
    size_t i, j;

    for (i = 0; i < cmd->argc; i++) {
        if (has_wildcard(cmd->argv[i])) {
            char **expanded = NULL;
            size_t expanded_count = 0;

            if (expand_token(cmd->argv[i], &expanded, &expanded_count) != 0) {
                free_string_array(new_argv, new_argc);
                return -1;
            }

            for (j = 0; j < expanded_count; j++) {
                if (add_string(&new_argv, &new_argc, expanded[j]) != 0) {
                    free_string_array(expanded, expanded_count);
                    free_string_array(new_argv, new_argc);
                    return -1;
                }
            }

            free_string_array(expanded, expanded_count);
        } else {
            if (add_string(&new_argv, &new_argc, cmd->argv[i]) != 0) {
                free_string_array(new_argv, new_argc);
                return -1;
            }
        }
    }

    new_argv = realloc(new_argv, (new_argc + 1) * sizeof(char *));
    if (new_argv == NULL) {
        free_string_array(new_argv, new_argc);
        return -1;
    }
    new_argv[new_argc] = NULL;

    for (i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);

    cmd->argv = new_argv;
    cmd->argc = new_argc;
    return 0;
}

int expand_job(job_t *job) {
    size_t i;

    for (i = 0; i < job->count; i++) {
        if (expand_command(&job->commands[i]) != 0) {
            return -1;
        }
    }

    return 0;
}