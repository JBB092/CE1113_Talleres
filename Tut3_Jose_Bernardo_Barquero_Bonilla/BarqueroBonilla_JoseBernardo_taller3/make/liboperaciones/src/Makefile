CC = gcc
CFLAGS = -Wall -I../include

.PHONY: all clean

all: ../bin/calculadora_e ../bin/calculadora_d

../bin/calculadora_e: main.c
	mkdir -p ../bin
	$(CC) $(CFLAGS) $< ../lib/liboperaciones.a -lm -o $@

../bin/calculadora_d: main.c
	mkdir -p ../bin
	$(CC) $(CFLAGS) $< -L ../lib -loperaciones -Wl,-rpath,'$$ORIGIN/../lib' -lm -o $@

clean:
	rm -f ../bin/calculadora_e ../bin/calculadora_d