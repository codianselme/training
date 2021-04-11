all: client.o server.o server client


client.o: client.c
	gcc -c  -Wall client.c -o client.o

server.o: server.c
	gcc -c  -Wall server.c -o server.o

server: server.o
	gcc -g server.o -o server 

client: client.o
	gcc -g client.o -o client
	

clean:
	rm *.o
	rm client
	rm server
