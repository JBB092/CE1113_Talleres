CC = gcc
AR = ar

.PHONY: all lib src clean

all: lib src

lib:
	$(MAKE) -C lib

src: lib
	$(MAKE) -C src

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C src clean