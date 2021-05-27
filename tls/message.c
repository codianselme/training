#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <picotls.h>

#include "message.h"

#define SL sizeof(long)

static double start_time;
static double nb_bytes_sent;
static int nb_bytes_recv;
static int sent_dataMB;
static int recv_dataMB;
static double last_sent_time;
static double last_recv_time;
static int recv_log;
static int sent_log;
static char logger[40][50];
static int logger_iter = 0; 

static int log_data(int fd, double nbytes, int *nMB, double *lasttime);

int msg_receive(int from, struct message *m){
  int ret;
  char *buff = malloc(sizeof(struct message));
  ret = recv(from, buff, sizeof(struct message), 0);
  if(ret < 0)
    return (E_IO);
  memset(m, 0, sizeof(*m));
  memcpy(&m->opcode,         buff, SL);
  memcpy(&m->result,         buff+SL, SL);
  memcpy(&m->params_len,     buff+SL+SL, SL);
  memcpy(&m->result_str_len, buff+SL+SL+SL, SL);
  memcpy(&m->params,         buff+SL+SL+SL+SL, m->params_len);
  memcpy(&m->result_str,     buff+SL+SL+SL+SL+m->params_len, m->result_str_len);
  nb_bytes_recv += ret;
  log_data(recv_log, nb_bytes_recv, &recv_dataMB, &last_recv_time);
  free(buff);
  return OK;
}

int msg_send(int to, struct message *m){
  char *buff = malloc(sizeof(struct message));
  memcpy(buff, &m->opcode, SL);
  memcpy(buff+SL, &m->result, SL);
  memcpy(buff+SL+SL, &m->params_len, SL);
  memcpy(buff+SL+SL+SL, &m->result_str_len, SL);
  memcpy(buff+SL+SL+SL+SL, m->params, m->params_len);
  memcpy(buff+SL+SL+SL+SL+m->params_len, m->result_str, m->result_str_len);
  
  int ret = send(to, buff, sizeof(struct message),0);
  fprintf(stderr, "old send: %ld:%ld:%ld:%ld:%s:%s\n", m->opcode, m->result, m->params_len, m->result_str_len, m->params, m->result_str);
  if(ret < 0)
    return E_IO;
  nb_bytes_sent += 2;
  log_data(sent_log, nb_bytes_sent, &sent_dataMB, &last_sent_time);
  free(buff);
  return OK;
}

int ptls_msg_send(int to, struct message *m, ptls_t *tls){
  int ret;
  uint8_t *buff = malloc(sizeof(struct message));
  ptls_buffer_t sendbuf;
  ptls_buffer_init(&sendbuf, "", 0);
  memcpy(buff, &m->opcode, SL);
  memcpy(buff+SL, &m->result, SL);
  memcpy(buff+SL+SL, &m->params_len, SL);
  memcpy(buff+SL+SL+SL, &m->result_str_len, SL);
  memcpy(buff+SL+SL+SL+SL, m->params, m->params_len);
  memcpy(buff+SL+SL+SL+SL+m->params_len, m->result_str, m->result_str_len);
  ret = ptls_send(tls, &sendbuf, buff, sizeof(struct message));
  assert(ret == 0);
  //send_fully(to, sendbuf.base, sendbuf.off);
  
  ret = send(to, sendbuf.base, sendbuf.off,0);
  fprintf(stderr, "sending: %ld:%ld:%ld:%ld:%s:%s:%ld:%ld\n", m->opcode, m->result, m->params_len, m->result_str_len, m->params, m->result_str, sendbuf.off, sizeof(struct message));
  assert(ret == sendbuf.off);
  if(ret < 0)
    return E_IO;
  nb_bytes_sent += 2;
  log_data(sent_log, nb_bytes_sent, &sent_dataMB, &last_sent_time);
  ptls_buffer_dispose(&sendbuf);
  free(buff);
  return OK;
}


int ptls_msg_receive(int from, struct message *m, ptls_t *tls){
  int ioret, ret;
  size_t input_off = 0, input_size = sizeof(struct message);
  ptls_buffer_t plaintextbuf;
  uint8_t *buff = malloc(input_size);
  memset(m, 0, sizeof(struct message));
  ioret = recv(from, buff, TLS_RECORD_SIZE, 0);
  if(ioret < 0)
    return (E_IO);
  if (ioret == 0)
    return 0;
  ptls_buffer_init(&plaintextbuf, "", 0);
  do{
    size_t consumed = ioret - input_off;
    ret = ptls_receive(tls, &plaintextbuf, buff + input_off, &consumed);
    input_off += consumed;
    fprintf(stderr, "start of receive (%d) (%d) (%ld) (%ld)\n", ioret, ret, consumed, plaintextbuf.off);
  } while (ret == 0 && input_off < input_size);
  assert(ret == 0);
  memset(m, 0, input_size);
  memcpy(&m->opcode,         (struct message*)plaintextbuf.base, SL);
  memcpy(&m->result,         (struct message*)plaintextbuf.base+SL, SL);
  memcpy(&m->params_len,     plaintextbuf.base+SL+SL, SL);
  memcpy(&m->result_str_len, plaintextbuf.base+SL+SL+SL, SL);
  memcpy(&m->params,         plaintextbuf.base+SL+SL+SL+SL, m->params_len);
  memcpy(&m->result_str,     plaintextbuf.base+SL+SL+SL+SL+m->params_len, m->result_str_len);
  nb_bytes_recv += plaintextbuf.off;
  log_data(recv_log, nb_bytes_recv, &recv_dataMB, &last_recv_time);
  fprintf(stderr, "start of receive (%d) (%ld)\n", ret, m->opcode);
  free(buff);
  ptls_buffer_dispose(&plaintextbuf);
  fprintf(stderr, "end of receive\n");
  return input_off;
}

double gettime_ms(void){
  struct timeval tv1;
  gettimeofday(&tv1, NULL);
  return (tv1.tv_sec * 1000000 + tv1.tv_usec);
}

void init_params(int recv_log_, int sent_log_){
  start_time = gettime_ms();
  nb_bytes_sent = 0;
  nb_bytes_recv = 0;
  sent_dataMB = 0;
  recv_dataMB = 0;
  last_sent_time = 0;
  last_recv_time = 0;
  recv_log = recv_log_;
  sent_log = sent_log_;
}

static int log_data(int fd, double nbytes, int *nMB, double *lasttime){
  int ret = -1;
  if((int)(nbytes/10000000) > *nMB){ 
    (*nMB)++;
    double delta_time = (gettime_ms() - start_time)/1000;
    double bw = 10*1000/(delta_time - *lasttime);
    ret = dprintf(fd, "%.6f %.6f\n", delta_time/1000, bw);
    //ret = snprintf(logger[logger_iter++], 50, "%.6f %.6f\n", delta_time/1000, bw);
    *lasttime = delta_time;
  }
  return ret;
}

int set_recv_data(int recv_data){
  nb_bytes_recv = recv_data;
  return log_data(recv_log, nb_bytes_recv, &recv_dataMB, &last_recv_time);
}

int print_recv_log(void){
  int ret = 0;
  for(int i = 0; i < logger_iter; i++)
    ret += dprintf(recv_log, "%s", logger[i]);
  return ret;
}

int print_sent_log(void){
  int ret = 0;
  for(int i = 0; i < logger_iter; i++)
    ret += dprintf(sent_log, "%s", logger[i]);
  return ret;
}

int handle_msg(struct message *m_in, struct message *m_out, int data_sd){
  int fd;
  //char buffer[BUF_SIZE];
  int ret = -1;
  memset(m_out, 0, sizeof(struct message));
  switch(m_in->opcode){
    case GET:
      fprintf(stderr, "params(%s)\n", m_in->params);
      fd = open(m_in->params, O_RDONLY);
      if(fd < 0){
        fprintf(stderr, "Error when opening (%s)\n", m_in->params);
        m_out->result = E_IO;
        strcpy(m_out->result_str, strerror(errno));
        m_out->result_str_len = strlen(m_out->result_str);
        return E_IO;
      }
      strcpy(m_out->result_str, strerror(errno));
      m_out->result_str_len = strlen(m_out->result_str);
      m_out->result = OK;
      return fd;
    default:
      fprintf(stderr, "Unkown commande");
      strcpy(m_out->result_str, "Server: Unkown command");
      m_out->result_str_len = strlen("Server: Unkown command");
      m_out->result = E_BAD_PARAM;
      break;
  }
  return ret; 
}
