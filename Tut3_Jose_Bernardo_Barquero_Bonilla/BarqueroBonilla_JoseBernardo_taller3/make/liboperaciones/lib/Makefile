CC = gcc
AR = ar
LIB = liboperaciones
CFLAGS = -Wall -fPIC -I../include

.PHONY: all clean

all: $(LIB).a $(LIB).so

calculadora.o: calculadora.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB).a: calculadora.o
	$(AR) rcs $@ $^

$(LIB).so: calculadora.o
	$(CC) -shared -o $@ $^ -lm

clean:
	rm -f calculadora.o $(LIB).a $(LIB).so