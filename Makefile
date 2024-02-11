NAME					:=	ircserv
CC						:=	c++
CFLAGS				:=	-Wall -Wextra -Werror \
									-std=c++14

rwildcard			=		$(strip $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d)))

SRCS					:=	$(strip $(call rwildcard,src/,*.cpp))
OBJS					:=	$(strip $(patsubst src/%, obj/%, $(SRCS:.cpp=.o)))

.PHONY: all
all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(dir $<) -Isrc/common -c -o $@ $<

.PHONY: clean
clean:
	rm -rf obj/

.PHONY: fclean
fclean: clean
	rm -f $(NAME)

.PHONY: re
re: fclean all

.PHONY: makefile-debug
makefile-debug:
	$(info NAME=$(NAME))
	$(info CC=$(CC))
	$(info CFLAGS=$(CFLAGS))
	$(info SRCS=$(SRCS))
	$(info OBJS=$(OBJS))
