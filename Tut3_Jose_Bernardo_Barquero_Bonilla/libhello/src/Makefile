CC = gcc
CFLAGS = -Wall -I../include

.PHONY: all clean

all: ../bin/saludo_e ../bin/saludo_d

../bin/saludo_e: main.c
	mkdir -p ../bin
	$(CC) $(CFLAGS) $< ../lib/libhello.a -o $@

../bin/saludo_d: main.c
	mkdir -p ../bin
	$(CC) $(CFLAGS) $< -L ../lib -lhello -o $@

clean:
	rm -f ../bin/saludo_e ../bin/saludo_d
