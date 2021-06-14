LD_FLAGS=-lreadline -lpicohttp-core -lpicoquic-core -lpicoquic-log -lpicotls-core -lpicotls-fusion -lpicotls-minicrypto -lpicotls-openssl  -lssl -lcrypto -lpthread
LD_PATH=-L./picoquic -L./picotls
INCLUDE_PATH=-I./picoquic/picoquic -I./picoquic/loglib/ 
all: client.o server.o server client


client.o: client.c
	gcc -c  -Wall client.c -o client.o $(INCLUDE_PATH)

server.o: server.c
	gcc -c  -Wall server.c -o server.o $(INCLUDE_PATH) 

server: server.o
	gcc -g server.o -o server $(LD_PATH) $(LD_FLAGS)

client: client.o
	gcc -g client.o -o client  $(LD_PATH) $(LD_FLAGS)
	

clean:
	rm *.o
	rm client
	rm server
