CC = gcc
CFLAGS = -std=c99 -Werror

all: 

assembler: assembler.c
	$(CC) -o assembler assembler.c