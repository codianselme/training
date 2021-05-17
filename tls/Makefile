LD_FLAGS=-lreadline -lpicotls-core -lpicotls-fusion -lpicotls-minicrypto -lpicotls-openssl -lssl -lcrypto
LD_PATH=-L./picotls
INCLUDE_PATH=-I./picotls/include
all: utils.o client.o server.o server client

utils.o: utils.c
	gcc -c  -Wall utils.c -o utils.o

message.o: message.c
	gcc -c  -Wall message.c -o message.o $(INCLUDE_PATH)

client.o: client.c
	gcc -c  -Wall client.c -o client.o $(INCLUDE_PATH)

server.o: server.c
	gcc -c  -Wall server.c -o server.o $(INCLUDE_PATH) 

server: utils.o message.o server.o
	gcc -g utils.o message.o server.o -o server $(LD_PATH) $(LD_FLAGS)

client: utils.o client.o
	gcc -g utils.o message.o client.o -o client  $(LD_PATH) $(LD_FLAGS)
	

clean:
	rm *.o
	rm client
	rm server
