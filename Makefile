NAME					:=	ircserv
CC						:=	c++

# Flags used by default
CFLAGS				:=	-Wall -Wextra -Werror \
									-std=c++14

# Flags used by default, except with "make release"
DFLAGS				:=	-DDEBUG \
									-g3 \
									-Wconversion \
									-Wdouble-promotion

# Flags used with DFLAGS, except with "make lldb"
SFLAGS				:=	-fsanitize=address,undefined,unreachable,null

# Flags used only with "make release"
RFLAGS				:=	-O3

rwildcard			=		$(strip $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d)))

SRCS					:=	$(strip $(call rwildcard,src/,*.cpp))
OBJS					:=	$(strip $(patsubst src/%, obj/%, $(SRCS:.cpp=.o)))

.PHONY: all
all: CFLAGS += $(DFLAGS)
all: CFLAGS += $(SFLAGS)
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(dir $<) -Isrc/common -c -o $@ $<

.PHONY: clean
clean:
	rm -rf obj/
	rm -rf $(NAME).dSYM

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

.PHONY: nuke
nuke:
	git clean -dxf
	git reset --hard

.PHONY: makefile-debug
makefile-debug:
	$(info NAME=$(NAME))
	$(info CC=$(CC))
	$(info CFLAGS=$(CFLAGS))
	$(info SRCS=$(SRCS))
	$(info OBJS=$(OBJS))

.PHONY: run
run: re
	./$(NAME) 6667 horse

.PHONY: release
release: CFLAGS += $(RFLAGS)
release: fclean $(NAME)

.PHONY: lldb
lldb: CFLAGS += $(DFLAGS)
lldb: fclean $(NAME)
