# Author: Edward Ly
# Last Modified: 14 December 2016
# Makefile for chat client and server

INCLUDE_FILES = c-s-socket.h
BUILD_FILES = client.c server.c
TARGET_FILES = client server
LIBRARIES = -lpthread

CC = gcc -O0 -Wall -ggdb

all: $(BUILD_FILES) $(INCLUDE_FILES)
	$(CC) client.c -o client $(LIBRARIES)
	$(CC) server.c -o server $(LIBRARIES)

client: client.c $(INCLUDE_FILES)
	$(CC) client.c -o client $(LIBRARIES)

server: server.c $(INCLUDE_FILES)
	$(CC) server.c -o server $(LIBRARIES)

clean:
	rm -f $(TARGET_FILES)
