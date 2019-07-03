all: server client

server: server.c
	gcc -c server.c
	gcc -o serverExe server.o -lm -lpthread

client: client.c
	gcc -c client.c
	gcc -o clientExe client.o

clean:
	rm serverExe clientExe *.o
