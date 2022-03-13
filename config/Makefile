CC=c++
CFLAGS=-Wall -Wextra -Werror -std=c++98 -g -DDEBUG

all: $(NAME)
	$(CC) $(CFLAGS) *.cpp -o a.out

clean:
	rm -rf *.o a.out

re: clean all

,PHONY: clean all re
