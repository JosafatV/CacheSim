SRC_DIR = src/
LIB_DIR = lib/

all: 
		mkdir -p bin && make -C $(LIB_DIR) && make -C $(SRC_DIR)

run:
		bin/main

reset:
		rm -r bin
		rm lib/*.o
		rm lib/*.a
		clear
