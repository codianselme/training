#include <stdint.h>
#include <stdio.h>
#include <picoquic.h>
#include <picosocks.h>
#include <picoquic_utils.h>
#include <autoqlog.h>
#include "picoquic_packet_loop.h"


typedef struct st_sample_server_stream_ctx_t {
    struct st_sample_server_stream_ctx_t* next_stream;
    struct st_sample_server_stream_ctx_t* previous_stream;
    uint64_t stream_id;
    FILE* F;
    uint8_t file_name[256];
    size_t name_length;
    size_t file_length;
    size_t file_sent;
    unsigned int is_name_read : 1;
    unsigned int is_stream_reset : 1;
    unsigned int is_stream_finished : 1;
} sample_server_stream_ctx_t;

typedef struct st_sample_server_ctx_t {
    char const* default_dir;
    size_t default_dir_len;
    sample_server_stream_ctx_t* first_stream;
    sample_server_stream_ctx_t* last_stream;
} sample_server_ctx_t;

sample_server_stream_ctx_t * sample_server_create_stream_context(sample_server_ctx_t* server_ctx, uint64_t stream_id);
int sample_server_open_stream(sample_server_ctx_t* server_ctx, sample_server_stream_ctx_t* stream_ctx);
void sample_server_delete_stream_context(sample_server_ctx_t* server_ctx, sample_server_stream_ctx_t* stream_ctx);
void sample_server_delete_context(sample_server_ctx_t* server_ctx);
int sample_server_callback(picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);
int picoquic_sample_server(int server_port, const char* server_cert, const char* server_key, const char* default_dir);
