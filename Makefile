NAME	:= httpserver

D_SRCS	:= src
D_OBJS	:= obj

SRC		:= main.cpp ByteBuffer.cpp Client.cpp HTTPMessage.cpp HTTPRequest.cpp \
		   HTTPResponse.cpp ResourceHost.cpp Server.cpp Resource.cpp

SRCS	:= $(addprefix, $(D_SRCS)/, $(SRC))
OBJS	:= $(addprefix $(D_OBJS)/, $(SRC:.cpp=.o))

CC		:= clang++
CFLAGS  := -Wall -Wextra -Werror -g -std=c++98 -pthread # -fsanitize=address
RM		:= rm -rf

INC		:= -I$(D_INC)/

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(D_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(D_OBJS):
	mkdir -p $(D_OBJS)

$(D_OBJS)/%.o: $(D_SRCS)/%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) $(D_OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all