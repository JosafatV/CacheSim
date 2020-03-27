CC = gcc
SRC = main.c
OUT = ../bin/main

LIBLINKEDLIST = ../lib/lib_linked_list.a
LIBLOGGER = ../lib/libjvlogger.a

CFLAGS = -I../include
LDFLAGS = -L../lib 

main: $(SRC)
		$(CC) -o $(OUT) $(SRC) $(CFLAGS) $(LDFLAGS) $(LIBLINKEDLIST) $(LIBLOGGER) -pthread

clean:
		rm ../bin/main
		rm main.o
		clear
