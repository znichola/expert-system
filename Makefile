NAME	= expert-system 

CC		= clang++
CFLAGS	= -Wall -Wextra
CFLAGS	+= -Werror
CFLAGS	+= -std=c++17 #-pedantic

ifdef DEBUG
CFLAGS	+= -g3 -fsanitize=address
else
ifdef DEBUGL
CFLAGS	+= -g3
endif
endif

LEAKS_CHECK = valgrind

EXAMPLE_FILE = example_file.txt

FILES	= parser evaluator

MAIN_SRC	= srcs/main.cpp
MAIN_OBJ	= objs/main.o

OBJS_PATH = objs/
SRCS_PATH = srcs/
INCS_PATH = -Iincludes/.

SRCS	= $(addprefix $(SRCS_PATH), $(addsuffix .cpp, $(FILES)))
OBJS	= $(addprefix $(OBJS_PATH), $(addsuffix .o, $(FILES)))

all	: $(NAME)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $(INCS_PATH) -o $@ $<

$(NAME)	: $(OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(OBJS) $(MAIN_OBJ) -o $@

clean	:
	-rm $(OBJS) $(MAIN_OBJ)

fclean	: clean
	-rm $(NAME)

re	: fclean all

leaks : re
	$(LEAKS_CHECK) ./$(NAME) $(EXAMPLE_FILE)

run : all
	$(LEAKS_CHECK) ./$(NAME) $(EXAMPLE_FILE)

LIBNAME = libES.a

# Rule to archive object files into a static library.
# Useful for testing
$(LIBNAME): $(OBJS)
	ar rcs $(LIBNAME) $(OBJS)

