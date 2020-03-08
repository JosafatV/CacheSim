LINKED_LIST_LIB = lib_linked_list.a
LINKED_LIST_SRC = linked_list.c
LINKED_LIST_OBJ = linked_list.o

INC_DIR = ../include/
LIB_DIR=../lib/
CC = gcc -c -I$(INC_DIR)
AR = ar rvs

all: $(LINKED_LIST_LIB)

$(LINKED_LIST_LIB) : $(LINKED_LIST_OBJ)
		$(AR) $(LINKED_LIST_LIB) $(LINKED_LIST_OBJ)
$(LINKED_LIST_OBJ) : $(LINKED_LIST_SRC)
		$(CC) $(LINKED_LIST_SRC)

clean:
		rm *.o