CC = gcc
CFLAGS = -g -Wall -std=c99 -fsanitize=address -O2

mysh: mysh.o arraylist.o
	$(CC) $(CFLAGS)  mysh.o arraylist.o -o mysh

arraylist.o: arraylist.c arraylist.h
	$(CC) $(CFLAGS) -c arraylist.c

mysh.o: mysh.c arraylist.h
	$(CC) $(CFLAGS) -c mysh.c

clean: 
	rm *.o mysh