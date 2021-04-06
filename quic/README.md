### Compliation 
gcc server.c -lpicoquic-log -lpicoquic-core -lpicotls-core -lpicotls-openssl -lpicotls-fusion -lpicotls-minicrypto -ldl -lpthread -lssl -lcrypto -L. -I. -o server

### Run server
./server <port_number>

### Run client 
./client <port_number> <folder> <queried_file>

### PEM pass
PEM pass phrase = 'test'