.OBJECTS = $(wildcard src/*.c)
OBJECTS = $(.OBJECTS:src%.c=obj%.o)
CC=clang
CFLAGS=-std=c11 -Wall -Wextra -Werror -Wpedantic -g -D_XOPEN_SOURCE=700
NAME=requestsc

all: prepare $(NAME)

prepare:
	mkdir -p obj/

$(NAME): $(OBJECTS)
	ar -rcs $(NAME).a $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -Iinc -o $@ -c $<

clean:
	rm -rf obj

uninstall: clean
	rm -f $(NAME).a

reinstall: uninstall all

run: reinstall
	$(CC) $(CFLAGS) -Iinc -lcurl -o main __main.c $(NAME).a
