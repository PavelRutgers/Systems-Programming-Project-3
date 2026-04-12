#include "shell.h"
#include <unistd.h>

ssize_t shell_read_line(int fd, char *buffer, size_t max_len) {
    size_t i = 0;
    char ch;
    ssize_t bytes_read = 0;

    if (max_len == 0) {
        return -1;
    }

    while (i < max_len - 1) {
        bytes_read = read(fd, &ch, 1);
        if (bytes_read < 0) {
            return -1;
        }
        if (bytes_read == 0) {
            break;
        }
        if (ch == '\n') {
            break;
        }

        buffer[i++] = ch;
    }

    if (i == 0 && bytes_read == 0) {
        return 0;
    }

    buffer[i] = '\0';
    return (ssize_t)i;
}