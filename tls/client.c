#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <picotls/openssl.h>
#include <picotls.h>

#include "utils.h"
#include "message.h"


#define MAXLINE 200
#define MAX_PORT_SIZE 6
#define MAX_NAME_SIZE 255

static int TCP_Connect(int af, char *server_ipaddr, char *server_port);
int File_put(int sock_msg, int sock_file);
int File_get(int sock_msg, int sock_file);
int File_ls(int sock_msg, int sock_file);
int File_pwd(int sock_msg,int sock_file);
int File_cd(int sock_msg);
void Quit(int sock_msg);
void Help();
int Client_cd();
int Client_pwd();
int Client_ls();
void Screen_print();
void Message(void);


int main(int argc, char *argv[]){
  int cmd_sd, data_sd, ret;
  char port1[MAX_PORT_SIZE], port2[MAX_PORT_SIZE];
  char host[MAX_NAME_SIZE];
  char cmd[MAX_NAME_SIZE];
  char params [MAX_NAME_SIZE];
  char c;
  struct message m_out, m_in; 
  fd_set readfds;
  fd_set writefds;
  int fdmax;
  char *input = NULL;
  char buffer[BUF_SIZE];
  char cmd_rbuffer[BUF_SIZE];
  char data_rbuffer[BUF_SIZE];
  int should_send_cmd = 0, data_fd, ioret;
  /*tls send and recv buffer*/
  ptls_buffer_t recvbuf, sendbuf, cmd_encbuf, data_encbuf;
  ptls_handshake_properties_t cmd_hsprop = {{{{NULL}}}};
  ptls_handshake_properties_t data_hsprop = {{{{NULL}}}};
  
  /* Init tls send and recv buff*/
  ptls_buffer_init(&recvbuf, "", 0);
  ptls_buffer_init(&sendbuf, "", 0);
  ptls_buffer_init(&cmd_encbuf, "", 0); 
  ptls_buffer_init(&data_encbuf, "", 0); 
  ptls_key_exchange_algorithm_t *key_exchanges[128] = {NULL};
  ptls_cipher_suite_t *cipher_suites[128] = {NULL};
  enum { IN_HANDSHAKE, IN_1RTT, IN_SHUTDOWN } cmd_state, data_state = IN_HANDSHAKE;
  ptls_context_t cmd_ctx = 
      {ptls_openssl_random_bytes, &ptls_get_time, key_exchanges, cipher_suites};
  ptls_context_t data_ctx = 
      {ptls_openssl_random_bytes, &ptls_get_time, key_exchanges, cipher_suites};
  cmd_ctx.send_change_cipher_spec = 1;
  data_ctx.send_change_cipher_spec = 1;
  
  /* tls context*/
  ptls_t *cmd_tls = ptls_new(&cmd_ctx, 0);
  ptls_t *data_tls = ptls_new(&data_ctx, 0);
  
  /* Recupération des paramètres de configuration du client et du serveur*/
  while((c = getopt(argc, argv, "p:P:h:")) != -1) {
    switch(c){
      case 'p':
         strncpy(port1, optarg, MAX_PORT_SIZE);
        break;
      case 'P':
        strncpy(port2, optarg, MAX_PORT_SIZE);
        break;
      case 'h':
        strncpy(host, optarg, MAX_NAME_SIZE);
        break;
      default:
        break;  
    }
  }
  /* 
   * Vérification des paramètres de configuration du client et du serveur
   * Connexion au serveur
   * Negociation des paramètres de configuration
   */
  cmd_sd =  TCP_Connect(AF_INET, host, port1);
  data_sd = TCP_Connect(AF_INET, host, port2);
  if(cmd_sd < 0 || data_sd < 0){
    fprintf(stderr, "Echec de la tentative de connection au serveur\n");
    fprintf(stderr, "%s\n", strerror(errno));
  }
  fdmax = MAX(cmd_sd, data_sd);
  /*
   * Echange avec le serveur et l'utilisateur de façon interactive
   */
  fcntl(cmd_sd, F_SETFL, O_NONBLOCK );
  fcntl(data_sd, F_SETFL, O_NONBLOCK );
  if (key_exchanges[0] == NULL)
    key_exchanges[0] = &ptls_openssl_secp256r1;
  if (cipher_suites[0] == NULL) {
    size_t i;
    for (i = 0; ptls_openssl_cipher_suites[i] != NULL; ++i)
      cipher_suites[i] = ptls_openssl_cipher_suites[i];
  }
  if ((ret = ptls_handshake(cmd_tls, &cmd_encbuf, NULL, NULL, &cmd_hsprop)) != PTLS_ERROR_IN_PROGRESS) {
    fprintf(stderr, "ptls_handshake:%d\n", ret);
  }
  
  if ((ret = ptls_handshake(data_tls, &data_encbuf, NULL, NULL, &data_hsprop)) != PTLS_ERROR_IN_PROGRESS) {
    fprintf(stderr, "ptls_handshake:%d\n", ret);
  }
  
  while(1){
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(cmd_sd, &readfds);
    FD_SET(cmd_sd, &writefds);
    FD_SET(data_sd, &readfds);
    FD_SET(data_sd, &writefds);
    FD_SET(0, &readfds);
    ret = select(fdmax+1, &readfds, &writefds,NULL,NULL);
    if(FD_ISSET(0, &readfds)){
      input = readline(">");
      if(input!=NULL){
        add_history(input);
        sscanf(input, "%s %s\n", cmd, params);
        
        if(!strcmp(cmd, "get") || !strcmp(cmd, "GET")){
          /* Commande GET */
          if(!strcmp(params, "")){
            fprintf(stderr, "parametre obligatoire\n");
          }
          else{
            data_fd = open(params, O_CREAT | O_WRONLY);
            if(data_fd < 0){
              fprintf(stderr, "Erreur I/O: %s\n", strerror(errno));
              FD_CLR(0, &readfds);
              continue;
            }
            m_out.opcode = GET;
            m_out.params_len = strlen(params);
            m_out.result_str_len = 0;
            strcpy(m_out.params, params);
            should_send_cmd = 1;
            
          }
        } else{
          /* TODO  Commande PUT, etc*/
          fprintf(stderr, "commande inconnue\n");
        }
        memset(cmd, 0, MAX_NAME_SIZE);
        memset(params, 0, MAX_NAME_SIZE);
        free(input);
      }
    }
    if(FD_ISSET(cmd_sd, &readfds)){
      if(!ptls_handshake_is_complete(cmd_tls)){
        size_t off = 0, leftlen;
        while ((ioret = read(cmd_sd, cmd_rbuffer, sizeof(cmd_rbuffer))) == -1 && errno == EINTR)
          ;
        if (ioret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
          /* no data */
          ioret = 0;
        } else if (ioret <= 0) {
          /* Quelque chose de grave vient de se produire*/
        }
        while ((leftlen = ioret - off) != 0) {
          if ((ret = ptls_handshake(cmd_tls, &cmd_encbuf, cmd_rbuffer + off, &leftlen, &cmd_hsprop)) == 0) {
            fprintf(stderr, "cmd_tls handshake ok\n");
            cmd_state = IN_1RTT;
            assert(ptls_is_server(cmd_tls) || 
            cmd_hsprop.client.early_data_acceptance != PTLS_EARLY_DATA_ACCEPTANCE_UNKNOWN);
            /* release data sent as early-data, if server accepted it */
            /* if (hsprop.client.early_data_acceptance == PTLS_EARLY_DATA_ACCEPTED)
               shift_buffer(&ptbuf, early_bytes_sent);*/
            /*if (request_key_update)
               ptls_update_key(tls, 1);*/
            fprintf(stderr, "in handshake haha\n");
          } else if (ret == PTLS_ERROR_IN_PROGRESS) {
                 /* ok */
          } else {
             if (cmd_encbuf.off != 0)
               (void)write(cmd_sd, cmd_encbuf.base, cmd_encbuf.off);
             fprintf(stderr, "ptls_handshake:%d\n", ret);
            }
            off += leftlen;
        }
        continue;
      }
      ptls_msg_receive(cmd_sd, &m_in, cmd_tls);
      /* TODO il faut vérifier que m_in est valide */
      fprintf(stderr, "%s\n", m_in.result_str);
      FD_CLR(cmd_sd, &readfds);
    }
    if(FD_ISSET(cmd_sd, &writefds)){
      if(should_send_cmd){
        /* TODO il faut verifier que m_out est valide */
        ptls_msg_send(cmd_sd, &m_out, cmd_tls);
        should_send_cmd = 0;
      }
      if(cmd_encbuf.off != 0){
         fprintf(stderr, "should send enc data (%ld)\n", cmd_encbuf.off);
         while((ret = send(cmd_sd, cmd_encbuf.base, cmd_encbuf.off,0))== -1 && (errno == EINTR))
           ;
         if(ret > 0)
           cmd_encbuf.off-= ret;
         if(ret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)){
           /* pas de données ecrite */
         }else{
           /* un evènement grave vient de se produire*/
         }
       }
      FD_CLR(cmd_sd, &writefds);
    }
    int val = 0;
    if(FD_ISSET(data_sd, &readfds)){
       if(!ptls_handshake_is_complete(data_tls)){
        size_t off = 0, leftlen;
        while ((ioret = read(data_sd, data_rbuffer, sizeof(data_rbuffer))) == -1 && errno == EINTR)
          ;
        if (ioret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
          /* no data */
          ioret = 0;
        } else if (ioret <= 0) {
          /* Quelque chose de grave vient de se produire*/
        }
        while ((leftlen = ioret - off) != 0) {
          if ((ret = ptls_handshake(data_tls, &data_encbuf, data_rbuffer + off, &leftlen, &data_hsprop)) == 0) {
            fprintf(stderr, "data_tls handshake ok\n");
            cmd_state = IN_1RTT;
            assert(ptls_is_server(data_tls) || 
            data_hsprop.client.early_data_acceptance != PTLS_EARLY_DATA_ACCEPTANCE_UNKNOWN);
            /* release data sent as early-data, if server accepted it */
            /* if (hsprop.client.early_data_acceptance == PTLS_EARLY_DATA_ACCEPTED)
               shift_buffer(&ptbuf, early_bytes_sent);*/
            /*if (request_key_update)
               ptls_update_key(tls, 1);*/
            fprintf(stderr, "in handshake haha\n");
          } else if (ret == PTLS_ERROR_IN_PROGRESS) {
                 /* ok */
          } else {
             if (data_encbuf.off != 0)
               (void)write(cmd_sd, data_encbuf.base, data_encbuf.off);
             fprintf(stderr, "ptls_handshake:%d\n", ret);
            }
            off += leftlen;
        }
        continue;
      }
      do{
         while((ret=recv(data_sd, buffer, BUF_SIZE,0)) < 0 && (errno==EINTR) );
         if(ret < 0){
           /* TODO il faut gérer */
         }
         if(ret > 0)
           val = write(data_fd, buffer, ret);
         if(ret == 0){
           fprintf(stderr, "Aucune donnée réçu %s (%d:%d)\n", strerror(errno), errno, val);
           /* TODO il faut gérer */
         }
       }while(ret>0);
       FD_CLR(data_sd, &readfds);
    }
    
    if(FD_ISSET(data_sd, &writefds)){
       /* TODO il faut gérer */
       if(data_encbuf.off != 0){
         fprintf(stderr, "should send enc data (%ld)\n", data_encbuf.off);
         while((ret = send(data_sd, data_encbuf.base, data_encbuf.off,0))== -1 && (errno == EINTR))
           ;
         if(ret > 0)
           data_encbuf.off-= ret;
         if(ret == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)){
           /* pas de données ecrite */
         }else{
           /* un evènement grave vient de se produire*/
         }
       }
       FD_CLR(data_sd, &writefds);
    }    
  }
}


void Message(void)
{
    printf("\n");
    printf("\n\t\t  **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**");
    printf("\n\t\t        =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf("\n\t\t        = get    put    pwd     ls     cd   quit    =");
    printf("\n\t\t        = client_cd     client_pwd      client_ls   =");
    printf("\n\t\t        =                  aide                     =");
    printf("\n\t\t        =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf("\n\t\t  **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**\n");
    printf("\n");
}

static int TCP_Connect(int af, char *server_ipaddr, char *server_port)
{
  int ret, sd;
  struct sockaddr server_addr;
  socklen_t salen;
  if ((sd = socket(af, SOCK_STREAM, 0)) < 0){
    return sd;
  }
  if(resolve_address(&server_addr, &salen, server_ipaddr, server_port, 
      AF_INET, SOCK_STREAM, IPPROTO_TCP)!= 0){
      fprintf(stderr, "Erreur de configuration de sockaddr\n");
      return -1;
  }
  if((ret = connect(sd, &server_addr, salen) ) < 0){
    fprintf(stderr, "Ici haha (%d) (%s:%s)\n", ret, strerror(errno), server_port);
    return ret;
  }
  fprintf(stderr, "con ret %d:%s:%d\n", ret, server_port,sd);
  return sd;
}



int File_put(int sock_msg,int sock_file){
	char filename[MAXLINE];
	int filehandle;
	//char buf[20];
	struct stat obj;
	int size;
	int value = 0;
	int status = 0;

	send(sock_msg, "put", 100,0);
	Screen_print();
	scanf("%s",filename);
	filehandle = open(filename, O_RDONLY);
	if(filehandle == -1){
		printf(" Il n'y a pas de fichier.\n");
		return -1;
	}

	send(sock_msg,filename,100,0);
	stat(filename,&obj);
	size = obj.st_size;
	send(sock_msg, &size, sizeof(int), 0);
	sendfile(sock_file, filehandle, NULL, size);
	if(status){
		printf("Téléchargement complet\n");
	}
	else{
		printf("Échec du téléchargement\n");
	}
	return value;
}




int File_get(int sock_msg,int sock_file){
	//char buf[50];
	char filename[MAXLINE];
	char temp[MAXLINE];
	int size;
	int value = 0;
	char *f;
	int filehandle;

	send(sock_msg, "get", 100,0);
	Screen_print();
	scanf("%s",filename);
	printf("Fichier : %s\n",filename);
	strcpy(temp,filename);
	//Envoyer le nom du fichier
	send(sock_msg, temp, 50, 0);
	//Reception de la taille du fichier
	recv(sock_msg, &size, sizeof(int), 0);
	//Lorsque la taille du fichier est de 0
	if(!size){
		printf("Il n'y a pas de fichier\n");
		return -1;
	}
	f = malloc(size);
	printf("Taille : %d octets\n",size);
	//Reception du fichier
	recv(sock_file,f,size,0);
	//Création de fichier
	while(1){
		filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0777);
		if(filehandle == -1){
			sprintf(filename + strlen(filename), "_1");
		}
		else{
			break;
		}
	}
	write(filehandle,f,size);
	close(filehandle);
	return value; 
}





int File_pwd(int sock_msg,int sock_file){
	int value;
	char buf[100];
	int size;
	int filehandle;
	char *f;

	strcpy(buf,"pwd");
	value = send(sock_msg,buf,100,0);
	value = recv(sock_msg, &size, sizeof(int), 0);
	f = malloc(size);
	recv(sock_file,f,size,0);
	filehandle = open("pwd.txt",O_RDWR | O_CREAT,0666);
	write(filehandle,f,size);
	close(filehandle);
	system("cat pwd.txt");
	return value; 
}




int File_ls(int sock_msg, int sock_file){
	char buf[100];
	int size;
	int filehandle;
	int value;
	char *f;

	strcpy(buf,"ls");
	printf("%s\n",buf);
	send(sock_msg, buf, 100,0);
	recv(sock_msg,&size,sizeof(int),0);
	f = malloc(size);
	value = recv(sock_file,f,size,0);
	filehandle = open("ls.txt",O_RDWR | O_CREAT ,0666);
	write(filehandle, f, size);
	close(filehandle);
	system("cat ls.txt");

	return value; 
}



int File_cd(int sock_msg){
	char buf[100];
	char temp[20];
	int value = 0;
	int status;

	send(sock_msg,"cd",100,0);
	strcpy(buf,"cd ");
	Screen_print();
	scanf("%s", temp);
	strcat(buf,temp);
	send(sock_msg,buf,100,0);
	recv(sock_msg, &status, sizeof(int), 0);

	if (status){
		printf(" Changement de chemin complet \n");
	}
	else{
		printf(" Le changement de chemin a échoué \n");
	}
	return value; 
}



void Quit(int sock_msg){
	system("rm ls.txt");
	system("rm pwd.txt");
	exit(0); 
}
	

int Client_pwd(){
	int result;
	result = system("pwd");
	return result;
}



int Client_cd(){
	int result;
	char input[100];
	strcpy(input,"cd ");
	Screen_print();
	scanf("%s", input + 3);
	if(chdir(input + 3) == 0){
		result = 0;
	}
	else{
		result = -1;
	}
	return result;
}


void Screen_print(){
	printf("client> ");
	//printf("cmd>");
}
	

int Client_ls(){
	int result;
	printf("Liste des fichiers \n");
	result = system("ls");
	return result;
}



void Help(){
	printf ("\033[1;33m Usage \tEX: get filename.extension\n");
	printf ("\033[1;33m <get>\t\tTélécharge un fichier du serveur vers le client. \n");
	printf ("\033[1;33m <put>\t\tTélécharge un fichier du client vers le serveur. \n");
	printf ("\033[1;33m <pwd>\t\tMontre le chemin vers le serveur. \n");
	printf ("\033[1;33m <ls>\t\tAffiche une liste de fichiers sur le serveur. \n");
	printf ("\033[1;33m <cd>\t\tDéplace le chemin vers le serveur. \n");
	printf ("\033[1;33m <quit>     \tArrête le serveur. \n");
	printf ("\033[1;33m <client_cd>  \tDéplace le chemin du client. \n");
	printf ("\033[1;33m <client_pwd> \tMontre le chemin du client. \n");
	printf ("\033[1;33m <client_ls>  \tAffiche une liste de fichiers sur le client. \n");
}
