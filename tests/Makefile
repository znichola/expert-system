CC		= g++
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

UNIT_TESTS = test_DS test_parser test_tokenizer

OBJS_PATH = objs/
SRCS_PATH = srcs/
INCS_PATH = -I../includes/.

LIB_ORIGIN_DIR = ../
LIB_NAME = ES
LIB_FILE_NAME = libES.a
LIB_PATH = ../

SRCS	= $(addprefix $(SRCS_PATH), $(addsuffix .cpp, $(UNIT_TESTS)))
OBJS	= $(addprefix $(OBJS_PATH), $(addsuffix .o,   $(UNIT_TESTS)))

all : run_all_tests

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $(INCS_PATH) -o $@ $<

$(UNIT_TESTS): % : $(OBJS_PATH)%.o $(LIB_ORIGIN_DIR)$(LIB_FILE_NAME)
	$(CC) $(CFLAGS) $^ -L$(LIB_PATH) -l$(LIB_NAME) -o $@

$(LIB_ORIGIN_DIR)$(LIB_FILE_NAME):
	$(MAKE) -s -C $(LIB_ORIGIN_DIR) $(LIB_FILE_NAME)

.PHONY: $(LIB_ORIGIN_DIR)$(LIB_FILE_NAME)

# Run all tests
run_all_tests: $(UNIT_TESTS)
	@for t in $(UNIT_TESTS); do \
		echo ">>> Running $$t"; \
		./$$t || exit 1; echo "";\
	done

run_%: %
	./$<

clean:
	-rm -f $(OBJS)

fclean: clean
	-rm -f $(UNIT_TESTS)
	$(MAKE) -C $(LIB_ORIGIN_DIR) fclean

re: fclean all

.PHONY: clean fclean re run_all_tests all
