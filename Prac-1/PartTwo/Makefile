all: Client.o Server.o
	gcc Client.o -o Client
	gcc Server.o -o Server
	rm Server.o Client.o

Client: Client.c
	gcc Client.c -o Client.o

Server: Server.c
	gcc Server.c -o Server.o

clean:
	rm Server Client