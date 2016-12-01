ID=ar267209
CC=gcc
CFLAGS=-std=c11 -Wall -Wextra -Wshadow -pedantic

TARGET=brighten
TEST_DIR=test

all: $(TARGET) 

brighten: brighten.c brighten.o
	$(CC) $(CFLAGS) -o $(TARGET) -lnetpbm brighten.c brighten.o

brighten.o: brighten.asm
	nasm -f elf64 -F dwarf -g brighten.asm

archive: clean Makefile brighten.c brighten.o
	tar czvf $(ID)-zad1.tar.gz *

test: clean $(TARGET)
	cd test && ./test.sh

clean:
	rm -rf *.o $(TARGET) $(ID)-zad1.tar.gz $(TEST_DIR)/test_*


.PHONY: clean test

