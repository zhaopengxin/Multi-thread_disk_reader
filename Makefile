CC=g++

CFLAGS=-g -ldl -pthread -std=c++11 -c -Wall


all: main
main: main.o DiskRequester.o
	$(CC) main.o DiskRequester.o libthread.o -ldl -pthread -std=c++11 -g -o runable
main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp libthread.o
DiskRequester.o: DiskRequester.cpp
	$(CC) $(CFLAGS) DiskRequester.cpp libthread.o
clean:
	rm -rf main.o  DiskRequester.o runable