CC=gcc -O3
#CC=gcc -g

DEPS=

SOURCES=elite-decode.c 

all: elite-decode elite-decode-v2 bin2asc

elite-decode: elite-decode.c $(DEPS)
	$(CC) -o $@ $^

elite-decode-v2: elite-decode-v2.c $(DEPS)
	$(CC) -o $@ $^

bin2asc: bin2asc.c $(DEPS)
	$(CC) -o $@ $^

.PHONY: clean

clean:
	rm -f *.o *~ elite-decode bin2asc
