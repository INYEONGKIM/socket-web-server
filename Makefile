CC = gcc
TARGET = server

$(TARGET) : server.o
	$(CC) -o $(TARGET) server.o

server.o : server.c
	$(CC) -c -o server.o server.c

clean:
	rm *.o server