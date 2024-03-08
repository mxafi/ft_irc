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
INCS					:=	$(strip $(call rwildcard,src/,*.h)) $(strip $(call rwildcard,src/,*.hpp))
OBJS					:=	$(strip $(patsubst src/%, obj/%, $(SRCS:.cpp=.o)))

TEST_SRCS			:=	$(strip $(call rwildcard,test/,*.cpp))
NOMAIN_SRCS		:=	$(filter-out src/main/main.cpp, $(SRCS))

.PHONY: all
all: CFLAGS += $(DFLAGS)
all: CFLAGS += $(SFLAGS)
all: $(NAME)

$(NAME): $(OBJS) $(INCS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(dir $<) -Isrc/common -c -o $@ $<

.PHONY: clean
clean:
	rm -rf obj/
	rm -rf $(NAME).dSYM
	rm -f test.out

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
	$(info TEST_SRCS=$(TEST_SRCS))
	$(info NOMAIN_SRCS=$(NOMAIN_SRCS))

.PHONY: run
run: all
	./$(NAME) 6667 horse

.PHONY: ports
ports:
	@echo "ircserv running on ports and sockets:"
	@lsof -i -P -n | grep "ircserv" | tr -s ' ' | cut -d' ' -f1,2,5,9,10

.PHONY: kill
kill:
	@lsof -i -P -n | grep "ircserv" | tr -s ' ' | cut -d' ' -f2 | sort | uniq | xargs kill -9

.PHONY: softkill
softkill:
	@lsof -i -P -n | grep "ircserv" | tr -s ' ' | cut -d' ' -f2 | sort | uniq | xargs kill

.PHONY: release
release: CFLAGS += $(RFLAGS)
release: fclean $(NAME)

.PHONY: lldb
lldb: CFLAGS += $(DFLAGS)
lldb: fclean $(NAME)

.PHONY: test
test: $(TEST_SRCS) $(NOMAIN_SRCS)
	$(CC) $(CFLAGS) -o test.out $(TEST_SRCS) $(NOMAIN_SRCS)
	./test.out $(ARGS)

.PHONY: git-clean-branches
git-clean-branches:
	git checkout main
	git fetch -p
	git branch --merged | grep -v "\*" | xargs -n 1 git branch -d
