CC = gcc -c
AR = ar rvs
RL = ranlib
NM = nm -s
LD = ld -o

LINKED_LIST = linked_list
LINKED_LIST_LIB = lib_linked_list.a
LINKED_LIST_SRC = linked_list.c
LINKED_LIST_OBJ = linked_list.o

LOGGER = jvlogger
LOGGER_SRC = $(LOGGER).c
LOGGER_OBJ = $(LOGGER).o
LOGGER_LIB = lib$(LOGGER).a

all: $(LINKED_LIST_SRC) $(LOGGER_SRC)
		make linked_list
		make jvlogger

linked_list:
		$(CC) $(LINKED_LIST_SRC)
		$(AR) $(LINKED_LIST_LIB) $(LINKED_LIST_OBJ)
		$(RL) $(LINKED_LIST_LIB)
		#$(NM) $(LINKED_LIST_LIB)

jvlogger:
		$(CC) $(LOGGER_SRC)
		$(AR) $(LOGGER_LIB) $(LOGGER_OBJ)
		$(RL) $(LOGGER_LIB)
		#$(NM) $(LOGGER_LIB)

clean:
		rm *.o