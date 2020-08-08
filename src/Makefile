CC=gcc -O3
#CC=gcc -g

DEPS=

SOURCES=elite-decode.c 

all: elite-decode

elite-decode: elite-decode.c $(DEPS)
	$(CC) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~ elite-decode
