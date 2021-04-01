gcc server.c -lpicoquic-log -lpicoquic-core -lpicotls-core -lpicotls-openssl -lpicotls-fusion -lpicotls-minicrypto -ldl -lpthread -lssl -lcrypto -L. -I. -o server


./server <port>

./client <port> <folder> <files_names>