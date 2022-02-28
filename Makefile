NAME	= webserv
CC		= c++
FLAGS	= -Wall -Wextra -Werror -std=c++98 -g
SRCDIR	= src
INCDIR	= inc
OBJDIR	= obj
SRC		= $(shell find src/*.cpp)
INC		= inc/main.hpp
OBJ		= $(subst $(SRCDIR), $(OBJDIR), $(SRC:cpp=o))

all: $(OBJDIR) $(OBJ)
	$(CC) $(FLAGS) -o $(NAME) $(OBJ)

$(NAME): all

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(FLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*

fclean: clean
	rmdir $(OBJDIR)
	rm -f $(NAME)

re: fclean all
