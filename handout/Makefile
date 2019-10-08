all: saber

CFLAGS=-g -Og -Wall -std=c99
CC=gcc

saber: saber.c rfid.o tag_data.o codec.c
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	rm -f saber
