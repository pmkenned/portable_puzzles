.PHONY: all run clean

NSOLVE ?= 1

CFLAGS = -Wall -Wextra
#CFLAGS = -O3
CFLAGS = -ggdb
CFLAGS += -DNSOLVE=$(NSOLVE)

all:
	gcc $(CFLAGS) main.c

run: all
	./a.out

clean:
	rm -f a.out
