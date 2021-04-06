#include <stdint.h>
#include <stdio.h>
#include <picoquic.h>
#include <picoquic_utils.h>
#include <picosocks.h>
#include <autoqlog.h>
#include <picoquic_packet_loop.h>


typedef struct st_sample_client_stream_ctx_t {
    struct st_sample_client_stream_ctx_t* next_stream;
    size_t file_rank;
    uint64_t stream_id;
    size_t name_length;
    size_t name_sent_length;
    FILE* F;
    size_t bytes_received;
    uint64_t remote_error;
    unsigned int is_name_sent : 1;
    unsigned int is_file_open : 1;
    unsigned int is_stream_reset : 1;
    unsigned int is_stream_finished : 1;
} sample_client_stream_ctx_t;

typedef struct st_sample_client_ctx_t {
    char const* default_dir;
    char const** file_names;
    sample_client_stream_ctx_t* first_stream;
    sample_client_stream_ctx_t* last_stream;
    int nb_files;
    int nb_files_received;
    int nb_files_failed;
    int is_disconnected;
} sample_client_ctx_t;

static int sample_client_create_stream(picoquic_cnx_t* cnx, sample_client_ctx_t* client_ctx, int file_rank);
static void sample_client_report(sample_client_ctx_t* client_ctx);
static void sample_client_free_context(sample_client_ctx_t* client_ctx);
int sample_client_callback(picoquic_cnx_t* cnx, uint64_t stream_id, uint8_t* bytes, size_t length, picoquic_call_back_event_t fin_or_event, void* callback_ctx, void* v_stream_ctx);
static int sample_client_loop_cb(picoquic_quic_t* quic, picoquic_packet_loop_cb_enum cb_mode, void* callback_ctx);
int picoquic_sample_client(char const* server_name, int server_port, char const* default_dir, int nb_files, char const** file_names);
