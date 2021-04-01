gcc server.c -lpicoquic-log -lpicoquic-core -lpicotls-core -lpicotls-openssl -lpicotls-fusion -lpicotls-minicrypto -ldl -lpthread -lssl -lcrypto -L. -I. -o server


### Generate the certificates
'''
openssl req -x509 -newkey rsa:2048 -days 365 -keyout ca-key.pem -out ca-cert.pem
openssl req -newkey rsa:2048 -keyout server-key.pem -out server-req.pem
'''

### Run the server
./picoquic_sample server 4433 ./ca-cert.pem ./server-key.pem ./server_files

### run the client
./picoquic_sample client localhost 4433 /tmp index.htm
