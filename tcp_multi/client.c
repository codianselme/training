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
  /*int pwd = 0, lpwd = 0, ls = 0, lls = 0, cd = 0, lcd = 0;
  char upload_file[MAX_NAME_SIZE];
  char download_file[MAX_NAME_SIZE];
  char local_dir[MAX_NAME_SIZE];
  char remote_dir[MAX_NAME_SIZE];*/
  fd_set readfds;
  fd_set writefds;
  int fdmax;
  char *input = NULL;
  char buffer[BUF_SIZE];
  int should_send_cmd = 0, data_fd;
  /* Recupération des paramètres de configuration du client et du serveur*/
  while((c = getopt(argc, argv, "p:P:h:u:d:cCs:S:")) != -1) {
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
      /*case 'u': *put file*
        strncpy(upload_file, optarg, MAX_NAME_SIZE);
        break;
      case 'd': *get file*
        strncpy(download_file, optarg, MAX_NAME_SIZE);
        break;
      case 'c': *remote current directory*
        pwd = 1;
        break;
      case 'C': * local current directory*
        lpwd = 1;
        break;*/
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
          fprintf(stderr, "commande inconnue\n");
        }
        memset(cmd, 0, MAX_NAME_SIZE);
        memset(params, 0, MAX_NAME_SIZE);
        free(input);
      }
    }
    if(FD_ISSET(cmd_sd, &readfds)){
      fprintf(stderr, "able to read on cmd_sd\n");
      msg_receive(cmd_sd, &m_in);
      fprintf(stderr, "%s\n", m_in.result_str);
      FD_CLR(cmd_sd, &readfds);
    }
    if(FD_ISSET(cmd_sd, &writefds)){
      //fprintf(stderr, "able to write on cmd_sd\n");
      if(should_send_cmd){
        msg_send(cmd_sd, &m_out);
        should_send_cmd = 0;
      }
      FD_CLR(cmd_sd, &writefds);
    }
    int val = 0;
    if(FD_ISSET(data_sd, &readfds)){
       fprintf(stderr, "able to read on data_sd\n");
       do{
         while((ret=recv(data_sd, buffer, BUF_SIZE,0)) < 0 && errno==EINTR );
         if(ret < 0){
           fprintf(stderr, "Erreur lors de la réception de données"
               "%s (%d:%d)\n", strerror(errno), errno, val);
         }
         if(ret > 0)
           val = write(data_fd, buffer, ret);
       }while(ret>0);
       FD_CLR(data_sd, &readfds);
    }
    
    if(FD_ISSET(data_sd, &writefds)){
       //fprintf(stderr, "able to write on data_sd\n");
      /* while((ret=recv(data_sd, buffer, BUF_SIZE,0)) > 0 )
         fprintf(stderr, "%s", buffer);
       FD_CLR(data_sd, &writefds);*/
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
