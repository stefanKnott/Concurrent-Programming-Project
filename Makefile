CC = gcc
CFLAGS = -c -Wall -Wextra

all: threading_suite

threading_suite: threading_suite.o
	$(CC) threading_suite.o -o threading_suite -lpthread

threading_suite.o: threading_suite.c
	$(CC) $(CFLAGS) threading_suite.c 

clean:
	rm -f threading_suite.o
