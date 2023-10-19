CC = g++
DEBUG = -g -O0 -pedantic-errors
CFLAGS = -no-pie -Wall -std=c++11 -c $(DEBUG)
LFLAGS = -no-pie -Wall $(DEBUG) 


all: myed


myed: main.o editorN.o 

	$(CC) $(LFLAGS) -o myed main.o editorN.o -lncurses

editorN.o: editorN.cpp editorN.h

	$(CC) $(CFLAGS) editorN.cpp

main.o: main.cpp editorN.h

	$(CC) $(CFLAGS) main.cpp


.PHONY: clean


clean: 

	rm -rf *.o
	rm -rf 1730ed
