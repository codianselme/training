#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picosocks.h>
#include "picoquic_sample.h"

static void usage(char const * sample_name)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s client server_name port folder *queried_file\n", sample_name);
    fprintf(stderr, "or :\n");
    fprintf(stderr, "    %s server port cert_file private_key_file folder\n", sample_name);
    exit(1);
}

int get_port(char const* sample_name, char const* port_arg)
{
    int server_port = atoi(port_arg);
    if (server_port <= 0) {
        fprintf(stderr, "Invalid port: %s\n", port_arg);
        usage(sample_name);
    }

    return server_port;
}
int main(int argc, char** argv)
{
    int exit_code = 0;
#ifdef _WINDOWS
    WSADATA wsaData = { 0 };
    (void)WSA_START(MAKEWORD(2, 2), &wsaData);
#endif

    if (argc < 2) {
        usage(argv[0]);
    }
    else if (strcmp(argv[1], "client") == 0) {
        if (argc < 6) {
            usage(argv[0]);
        }
        else {
            int server_port = get_port(argv[0], argv[3]);
            char const** file_names = (char const **)(argv + 5);
            int nb_files = argc - 5;
            exit_code = picoquic_sample_client(argv[2], server_port, argv[4], nb_files, file_names);
        }
    }
    else if (strcmp(argv[1], "server") == 0) {
        if (argc < 5) {
            usage(argv[0]);
        }
        else {
            int server_port = get_port(argv[0], argv[2]);
            exit_code = picoquic_sample_server(server_port, argv[3], argv[4], argv[5]);
        }
    }
    else
    {
        usage(argv[0]);
    }

    exit(exit_code);
}