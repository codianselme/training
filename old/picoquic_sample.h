/*
* Author: Christian Huitema
* Copyright (c) 2020, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdint.h>
#include <stdio.h>
#include <picoquic.h>
#include <picoquic_utils.h>
#include <picosocks.h>
#include <autoqlog.h>
#include <picoquic_packet_loop.h>


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


// typedef struct st_sample_server_stream_ctx_t {
//     struct st_sample_server_stream_ctx_t* next_stream;
//     struct st_sample_server_stream_ctx_t* previous_stream;
//     uint64_t stream_id;
//     FILE* F;
//     uint8_t file_name[256];
//     size_t name_length;
//     size_t file_length;
//     size_t file_sent;
//     unsigned int is_name_read : 1;
//     unsigned int is_stream_reset : 1;
//     unsigned int is_stream_finished : 1;
// } sample_server_stream_ctx_t;

// typedef struct st_sample_server_ctx_t {
//     char const* default_dir;
//     size_t default_dir_len;
//     sample_server_stream_ctx_t* first_stream;
//     sample_server_stream_ctx_t* last_stream;
// } sample_server_ctx_t;


// typedef struct st_sample_client_stream_ctx_t {
//     struct st_sample_client_stream_ctx_t* next_stream;
//     size_t file_rank;
//     uint64_t stream_id;
//     size_t name_length;
//     size_t name_sent_length;
//     FILE* F;
//     size_t bytes_received;
//     uint64_t remote_error;
//     unsigned int is_name_sent : 1;
//     unsigned int is_file_open : 1;
//     unsigned int is_stream_reset : 1;
//     unsigned int is_stream_finished : 1;
// } sample_client_stream_ctx_t;

// typedef struct st_sample_client_ctx_t {
//     char const* default_dir;
//     char const** file_names;
//     sample_client_stream_ctx_t* first_stream;
//     sample_client_stream_ctx_t* last_stream;
//     int nb_files;
//     int nb_files_received;
//     int nb_files_failed;
//     int is_disconnected;
// } sample_client_ctx_t;

// static int sample_client_create_stream(picoquic_cnx_t* cnx, sample_client_ctx_t* client_ctx, int file_rank);
// static void sample_client_report(sample_client_ctx_t* client_ctx);
// static void sample_client_free_context(sample_client_ctx_t* client_ctx);
// int sample_client_callback(picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);
// static int sample_client_loop_cb(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);


// sample_server_stream_ctx_t * sample_server_create_stream_context(sample_server_ctx_t* server_ctx, uint64_t stream_id);
// int sample_server_open_stream(sample_server_ctx_t* server_ctx, sample_server_stream_ctx_t* stream_ctx);
// void sample_server_delete_stream_context(sample_server_ctx_t* server_ctx, sample_server_stream_ctx_t* stream_ctx);
// void sample_server_delete_context(sample_server_ctx_t* server_ctx);
// int sample_server_callback(picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);


int picoquic_sample_client(char const* server_name, int server_port, char const* default_dir,
    int nb_files, char const** file_names);

int picoquic_sample_server(int server_port, const char* pem_cert, const char* pem_key, const char * default_dir);


