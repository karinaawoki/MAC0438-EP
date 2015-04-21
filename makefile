CC=gcc
CFLAGS=-Wall -pedantic -ansi -pthread -g
RM=rm -f

ep: main.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $^

clean:
	$(RM) *.o *.gch
