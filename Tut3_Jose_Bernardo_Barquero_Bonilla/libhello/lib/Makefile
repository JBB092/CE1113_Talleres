CC = gcc
AR = ar
CFLAGS = -Wall -fPIC -I../include
LIB = libhello

.PHONY: all clean

all: $(LIB).a $(LIB).so

say.o: say.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB).a: say.o
	$(AR) rcs $@ $^

$(LIB).so: say.o
	$(CC) -shared -o $@ $^

clean:
	rm -f say.o $(LIB).a $(LIB).so
