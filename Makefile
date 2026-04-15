CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
SRC = main.c input.c util.c tokenize.c parse.c exec.c path.c builtins.c wildcard.c 
OBJ = $(SRC:.c=.o)

mysh: $(OBJ)
	$(CC) $(CFLAGS) -o mysh $(OBJ)

clean:
	rm -f $(OBJ) mysh