NAME=a.out
CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98 -g -DDEBUG

all: re

$(NAME):
	$(CC) $(CFLAGS) -I. -I../inc *.cpp -o $@

clean:
	rm -rf $(NAME) *.o

re: clean $(NAME)

.PHONY: all clean re
