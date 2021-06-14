#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <picoquic.h>
#include <picoquic_utils.h>
#include <picosocks.h>
#include <autoqlog.h>
#include <picoquic_packet_loop.h>






int main(int argc, char** argv)
{
    picoquic_cnx_t* cnx = NULL;
    picoquic_get_initial_cnxid(cnx);
}
