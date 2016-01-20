CC = gcc
CFLAGS = -std=c99 -pthread -Wall -Werror

all: gen_tree

gen_tree: main.c
	$(CC) $(CFLAGS) $^ -o $@
	
clean:
	rm -f gen_tree

.PHONY: clean all
