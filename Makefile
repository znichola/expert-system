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
INCS_PATH = -Iincludes/.

SRCS	= $(addprefix $(SRCS_PATH), $(addsuffix .cpp, $(FILES)))
OBJS	= $(addprefix $(OBJS_PATH), $(addsuffix .o, $(FILES)))



GV_PLUGIN_DIR = $(HOME)/graphviz-static/lib/graphviz

GV_PLUGINS = \
    $(GV_PLUGIN_DIR)/libgvplugin_core.a \
    $(GV_PLUGIN_DIR)/libgvplugin_dot_layout.a \
    $(GV_PLUGIN_DIR)/libgvplugin_neato_layout.a \
    $(GV_PLUGIN_DIR)/libgvplugin_pango.a \
    $(GV_PLUGIN_DIR)/libgvplugin_gdk.a \
    $(GV_PLUGIN_DIR)/libgvplugin_kitty.a \
    $(GV_PLUGIN_DIR)/libgvplugin_xlib.a \
    $(GV_PLUGIN_DIR)/libgvplugin_vt.a

GV_LIBS = -L$(HOME)/graphviz-static/lib \
    -lgvc -lcgraph -lcdt -lxdot -lpathplan -lgvpr -lz -lm -lpng -lexpat \
    $(GV_PLUGINS)


all	: $(NAME)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $(INCS_PATH) -o $@ $<

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


# Rule to archive object files into a static library.
# Useful for testing
$(LIBNAME): $(OBJS)
	ar rcs $(LIBNAME) $(OBJS)

