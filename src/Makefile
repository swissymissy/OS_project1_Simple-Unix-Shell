# Makefile for rush shell

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = rush
OBJS = main.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c main.h
	$(CC) $(CFLAGS) -c main.c 

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean


