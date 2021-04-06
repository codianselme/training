#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picosocks.h>
#include <stdint.h>
#include <picoquic.h>
#include <picoquic_utils.h>
#include <picoquic_packet_loop.h>

#include "sample_client.h"
#include "sample_server.h"

#define PICOQUIC_SAMPLE_H
/* Header file for the picoquic sample project. 
 * It contains the definitions common to client and server */

#define PICOQUIC_SAMPLE_ALPN "picoquic_sample"
#define PICOQUIC_SAMPLE_SNI "test.example.com"

#define PICOQUIC_SAMPLE_NO_ERROR 0
#define PICOQUIC_SAMPLE_INTERNAL_ERROR 0x101
#define PICOQUIC_SAMPLE_NAME_TOO_LONG_ERROR 0x102
#define PICOQUIC_SAMPLE_NO_SUCH_FILE_ERROR 0x103
#define PICOQUIC_SAMPLE_FILE_READ_ERROR 0x104
#define PICOQUIC_SAMPLE_FILE_CANCEL_ERROR 0x105

#define PICOQUIC_SAMPLE_CLIENT_TICKET_STORE "sample_ticket_store.bin";
#define PICOQUIC_SAMPLE_CLIENT_TOKEN_STORE "sample_token_store.bin";
#define PICOQUIC_SAMPLE_CLIENT_QLOG_DIR ".";
#define PICOQUIC_SAMPLE_SERVER_QLOG_DIR ".";


// int picoquic_sample_server(int server_port, const char* pem_cert, const char* pem_key, const char * default_dir);

