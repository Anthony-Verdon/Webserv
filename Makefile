MAKEFLAGS += -j

NAME	:= webserv
SRCS	:= main.cpp\
		   Server.cpp\
		   Request.cpp\
		   parseRequest.cpp\
		   utils.cpp\
		   directoryListing.cpp\
		   statusCode.cpp
CC 		:= g++
CFLAGS  := -Wall -Wextra -Werror -std=c++98 -g -MMD
OBJS    := $(addprefix objs/, $(SRCS:.cpp=.o))

all: $(NAME)

run: all
	./${NAME}

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS)

objs/%.o: server/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf objs 

fclean: clean
	rm -f $(NAME)

re: fclean
	make

.PHONY: all clean fclean re

-include $(OBJS:.o=.d)
