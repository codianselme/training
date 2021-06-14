## Try picoquic

### Add submodule for picoquic repo

Create a new directory. cd to the new directory for example quic

```
$ mkdir quic && cd quic
```

Add submodule to link picoquic repo to your own repo
```
git submodule add https://github.com/private-octopus/picoquic.git
```

Add submodule to link picotls to your own repo

```
git submodule add https://github.com/h2o/picotls.git

```

Follow `picotls/README.md` guidelines to build picotls. Do  the same with
`picoquic/README.md` to build picoquic.


### Compliation 
gcc server.c -lpicoquic-log -lpicoquic-core -lpicotls-core -lpicotls-openssl -lpicotls-fusion -lpicotls-minicrypto -ldl -lpthread -lssl -lcrypto -L. -I. -o server

### Run server
./server <port_number>

### Run client 
./client <port_number> <folder> <queried_file>

### PEM pass
PEM pass phrase = 'test'
