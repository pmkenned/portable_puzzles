.PHONY: all run clean

NSOLVE ?= 1
MAX_DOMINO ?= 7

CFLAGS = -O3
CFLAGS += -DNSOLVE=$(NSOLVE)
CFLAGS += -DMAX_DOMINO=$(MAX_DOMINO)

all:
	gcc $(CFLAGS) main.c timer.c

run: all
	./a.out

clean:
	rm -f a.out
