#include "shell.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int add_token(token_list_t *tokens, const char *start, size_t len) {
    char *copy;
    char **new_items;
    size_t new_capacity;

    if (len == 0) {
        return 0;
    }

    if (tokens->size + 1 >= tokens->capacity) {
        new_capacity = (tokens->capacity == 0) ? 8 : tokens->capacity * 2;
        new_items = realloc(tokens->items, new_capacity * sizeof(char *));
        if (new_items == NULL) {
            return -1;
        }
        tokens->items = new_items;
        tokens->capacity = new_capacity;
    }

    copy = malloc(len + 1);
    if (copy == NULL) {
        return -1;
    }

    memcpy(copy, start, len);
    copy[len] = '\0';

    tokens->items[tokens->size] = copy;
    tokens->size++;
    tokens->items[tokens->size] = NULL;

    return 0;
}

int tokenize_line(const char *line, token_list_t *tokens) {
    size_t i;
    size_t start;

    tokens->items = NULL;
    tokens->size = 0;
    tokens->capacity = 0;

    i = 0;
    while (line[i] != '\0') {
        while (isspace((unsigned char) line[i])) {
            i++;
        }

        if (line[i] == '\0') {
            break;
        }

        if (line[i] == '#') {
            break;
        }

        if (line[i] == '<' || line[i] == '>' || line[i] == '|') {
            if (add_token(tokens, &line[i], 1) != 0) {
                free_tokens(tokens);
                return -1;
            }
            i++;
            continue;
        }

        start = i;
        while (line[i] != '\0' &&
               !isspace((unsigned char) line[i]) &&
               line[i] != '<' &&
               line[i] != '>' &&
               line[i] != '|' &&
               line[i] != '#') {
            i++;
        }

        if (add_token(tokens, &line[start], i - start) != 0) {
            free_tokens(tokens);
            return -1;
        }

        if (line[i] == '#') {
            break;
        }
    }

    return 0;
}

void free_tokens(token_list_t *tokens) {
    size_t i;

    for (i = 0; i < tokens->size; i++) {
        free(tokens->items[i]);
    }

    free(tokens->items);
    tokens->items = NULL;
    tokens->size = 0;
    tokens->capacity = 0;
}