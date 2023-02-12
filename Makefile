############## COMPILER ##############
cxx	= c++

################ FLAG ################
ifdef DEBUG
	FLAGS	:= -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address
else
	FLAGS	:= -Wall -Wextra -Werror -std=c++98 -O2
endif

############### TARGET ###############
NAME	= ircserv

################ FILE ################
HEADERS_DIR	= includes/
HEADERS_FILES	= Server.hpp User.hpp Channel.hpp Message.hpp Command.hpp FormatValidator.hpp CommonValue.hpp Bot.hpp
HEADERS	= $(addprefix $(HEADERS_DIR), $(HEADERS_FILES))

SRCS_DIR	= srcs/
SRCS_FILES	= main.cpp Server.cpp User.cpp Channel.cpp Message.cpp Command.cpp FormatValidator.cpp Bot.cpp
SRCS	= $(addprefix $(SRCS_DIR), $(SRCS_FILES))

################ OBJ #################
OBJS_DIR	= objs/
OBJS	= $(addprefix $(OBJS_DIR), $(SRCS_FILES:.cpp=.o))

############### Color ################
GREEN="\033[32m"
L_GREEN="\033[1;32m"
RED="\033[31m"
L_RED="\033[1;31m"
RESET="\033[0m"
BOLD="\033[1m"
L_PUPLE="\033[1;35m"
L_CYAN="\033[1;96m"
UP = "\033[A"
CUT = "\033[K"

################ RULE ################
all	: $(NAME)

$(OBJS_DIR):
	@mkdir $(OBJS_DIR)

$(OBJS): $(HEADER) | $(OBJS_DIR)

$(NAME)	: $(OBJS)
	@$(cxx) $(FLAGS) $^ -I$(HEADERS_DIR) -o $@
	@echo current complie FLAGS : $(FLAGS)
	@echo complete $(L_GREEN)COMPILE$(RESET) 🌸

$(addprefix $(OBJS_DIR), %.o): $(addprefix $(SRCS_DIR), %.cpp)
	@$(cxx) $(FLAGS) -c -I$(HEADERS_DIR) $< -o $@

clean	:
	@rm -rf $(OBJS_DIR)
	@echo $(L_RED)remove$(RESET) OBJ files 🌪

fclean : clean
	@rm -f $(NAME)
	@echo $(L_RED)remove$(RESET) $(NAME) a.k.a target file 🎯

re : fclean all

debug :
	@make DEBUG=1
	@echo start $(L_CYAN)DEBUG$(RESET) 😱 GOOD LUCK 🍀

.PHONY	: all clean fclean re debug