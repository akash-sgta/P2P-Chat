CC=gcc
LIBS=-lpthread

all: pack

pack: Server.o Client.o

Server.o: PROJECT_SERVER.c
	$(CC) -c -Wall PROJECT_SERVER.c $(LIBS) -o Server.o

Client.o: PROJECT_CLIENT.c
	$(CC) -c -Wall PROJECT_CLIENT.c $(LIBS) -o Client.o

clean:
	rm -rf *.o pack