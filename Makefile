NAME	= expert-system 

CC		= clang++
CFLAGS	= -Wall -Wextra
CFLAGS	+= -Werror
CFLAGS	+= -std=c++20 #-pedantic
CFLAGS	+=  -g3 -O0

ifdef DEBUG
CFLAGS	+= -g3 -fsanitize=address
else
ifdef DEBUGL
CFLAGS	+= -g3
endif
endif

LEAKS_CHECK = valgrind

EXAMPLE_FILE = example_file.txt

FILES	= parser expression token solver digraph server

MAIN_SRC	= srcs/main.cpp
MAIN_OBJ	= objs/main.o

OBJS_PATH = objs/
SRCS_PATH = srcs/
INCS_PATH = includes

SRCS	= $(addprefix $(SRCS_PATH), $(addsuffix .cpp, $(FILES)))
OBJS	= $(addprefix $(OBJS_PATH), $(addsuffix .o, $(FILES)))

GV_PREFIX = $(HOME)/graphviz

# Used to build the project with/without graphvis functionality
WITH_GRAPHVIZ := $(shell [ -f $(GV_PREFIX)/include/graphviz/gvc.h ] && echo 1 || echo 0)

ifeq ($(WITH_GRAPHVIZ),1)
    CFLAGS += -DWITH_GRAPHVIZ -I$(GV_PREFIX)/include
    GV_LIBS = -L$(GV_PREFIX)/lib -lgvc -lcgraph -lcdt -Wl,-rpath,$(GV_PREFIX)/lib
else
    GV_LIBS =
endif

all	: $(NAME)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -I$(INCS_PATH) -o $@ $<

$(NAME)	: $(OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $(OBJS) $(MAIN_OBJ) $(GV_LIBS) -o $@

clean	:
	-rm $(OBJS) $(MAIN_OBJ)

LIBNAME = libES.a

fclean	: clean
	-rm $(NAME) $(LIBNAME)

re	: fclean all

leaks : re
	$(LEAKS_CHECK) ./$(NAME) $(EXAMPLE_FILE)

run : all
	$(LEAKS_CHECK) ./$(NAME) $(EXAMPLE_FILE)

external_deps/graphviz-13.1.2 :
	mkdir -p external_deps
	cd external_deps && \
	wget https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/13.1.2/graphviz-13.1.2.tar.gz && \
	tar -xf graphviz-13.1.2.tar.gz && \
	rm graphviz-13.1.2.tar.gz

graphviz:
	cd external_deps/graphviz-13.1.2 && \
	./configure --prefix=$(GV_PREFIX) \
	&& \
	make && \
	make install

# Rule to archive object files into a static library.
# Useful for testing
$(LIBNAME): $(OBJS)
	ar rcs $(LIBNAME) $(OBJS)

.PHONY: all clean fclean re leaks run graphviz
