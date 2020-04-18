CC=gcc
LIBS=-pthread
SRC=src

all: server client
server: server.c
	$(CC) server.c $(LIBS) -o server

client: client.c
	$(CC) client.c $(LIBS) -o client


