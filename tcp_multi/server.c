#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "utils.h"
#include "message.h"

#define BACKLOG_SIZE 10
#define MAX_PORT_SIZE 6
#define MAX_HOST_NAME_SIZE 255


static int tcp_listen(char *server_ipaddr, char *server_port);
int tcp_ls(int sock_msg, int sock_file);
int tcp_get(int sock_msg, int sock_file);
int tcp_put(int sock_msg, int sock_file);
int tcp_pwd(int sock_msg, int sock_file);
int tcp_cd(int sock_msg);
void exitClient();
void main_select(int sock_msg, int sock_file);


int filehandle;
//

int num_user = 0;
pid_t pid;
pid_t pid1;
struct sigaction sact;
sigset_t mask;
struct sockaddr_in servaddr;

int main(int argc, char *argv[]){
  socklen_t len;
  struct sockaddr  client1, client2;
  char host[MAX_HOST_NAME_SIZE];
  /*
   * le serveur ecoute sur le port1 pour recevoir les commandes
   * le serveur écoute sur le port2 pour recevoir les donnees*/
  char port1[MAX_PORT_SIZE], port2[MAX_PORT_SIZE]; 
  
  /* sock1: est le socket sur lequel le serveur attent les connexions clientes
   *        pour les commandes
   * sock2: est le socket sur lequel le serveur attend les connexions clientes
   *        pour les données
   */
  int sock1, sock2;
  /*
   * cmd_sd: socket pour recevoir les commandes du client lorsque la connexion
   *         est établie
   * data_sd: socket pour recevoir les données du client lorsque la connexion
   *         est établi
   */
  int cmd_sd = -1, data_sd = -1;
  /* pid du processus qui sera créé pour gérer les connexions clientes */
  int child;
  /* caractère pour gérer les options en lignes de commande avec opts*/
  char c;
  /* descripteur du fichier à recevoir par la commande GET*/
  int data_fd = -1;
  /* Variable  pour  récupérer le resultat des appels systèmes*/
  int ret;
  /* structure de données pour gérer les entrées sortie async sur les socket 
   * sock1 et sock2. A l'aide de l'appel système select le serveur ne fait 
   * l'appel à accept que si il ya une connexion entrante
   */
  int fdmax = 0;
  fd_set readfds;
  fd_set writefds;
  
  /* should_send_cmd: (1) le serveur désire envoyer une commande (0) non
   * should_send_data: (1) le serveur désire envoyer des données (0) non
   */
  int should_send_cmd = 0, should_send_data = 0;
  /* mémoire tampon pour l'envoie et la réception*/
  char buffer[BUF_SIZE];
  /* message pour récupérer les requêtes et envoyer les réponses */
  struct message m_in, m_out;
  /* Gestion des options en ligne de commande */
  while((c = getopt(argc, argv, "p:P:h:")) != -1) {
    switch(c){
      case 'p':
         strncpy(port1, optarg, MAX_PORT_SIZE);
        break;
      case 'P':
        strncpy(port2, optarg, MAX_PORT_SIZE);
        break;
      case 'h':
        strncpy(host, optarg, MAX_HOST_NAME_SIZE);
        break;
      default:
        break;
      
    }
  }
  /* le serveur spécifie au système qu'il écoute les ports port1 et port2*/
  sock1 = tcp_listen(host, port1);
  sock2 = tcp_listen(host, port2);
  /* les sock1 et sock2 sont non bloquants cela signifie qu'il retourne lorsque
   * la ressource demandée lors d'un appel système n'est pas disponible sans
   * bloquer le processus
   */
  fcntl(sock1, F_SETFL, O_NONBLOCK );
  fcntl(sock2, F_SETFL, O_NONBLOCK );
  /* Initialisation des structures  de données readfds et writefds*/
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  fdmax = MAX(sock1, sock2);
  
  /* Le serveur entre dans une boucle en attente des connexions entrantes */
  while(1){
    /* Configuration des descripteurs de fichiers monitorés par le système à 
     * travers l'appel système select 
     */
    FD_SET(sock1, &readfds);
    FD_SET(sock2, &readfds);
    ret = select(fdmax+1, &readfds, NULL,NULL,NULL);
    /*
     * Une client tente de se connecter sur sock1
     */
    if(FD_ISSET(sock1, &readfds)){
      cmd_sd = accept(sock1, (struct sockaddr*)&client1, &len);
      FD_CLR(sock1, &readfds);
      /* Il faut attendre que sock2 soit prêt */
      if(cmd_sd > 0 && data_sd < 0)
        continue;
    }
    /* Un client tente de se connecter sur sock2 */
    if(FD_ISSET(sock2, &readfds)){
      data_sd = accept(sock2, (struct sockaddr*)&client2, &len);
      FD_CLR(sock2, &readfds);
      /* Il faut attendre que sock1 soit prêt */
      if(cmd_sd < 0 && data_sd > 0)
        continue;
    }
    /* On verifie que les connexion de commande et de données sont prêts */
    if(cmd_sd < 0 || data_sd < 0){
      fprintf(stderr, "failed to accept incomming connection (%s)\n", strerror(errno));
      exit(1);
    }
    
    if(cmd_sd>0 && data_sd>0){
       /* Le serveur a un client avec lequel dialogué pour cela il crée un fils*/
       child = fork();
       if(child < 0){
         exit(1);
       }
       if(child == 0){
         static int data_sent = 0; /**nbre d'octets dejà envoyés pas le serveur**/
         static int failed_send = 0; /** l'appel système send() a echoué **/
         static int send_length = 0; /** la taille des données à renvoyer suite à l'échec de send() **/
         int ds; /*nbre d'octets réellement envoyés par l'appel système send() */
         static int data_read = 0; /*nombre d'octets réellement lus dans  le fichier */
         static int offset = 0; /* offset dans le buffer d'envoie */
           
         /* Le serveur prépare les descripteur de fichier qui lui permettrons
          * de discuter avec le client 
          */
         fd_set creadfds;
         fd_set cwritefds;
         FD_ZERO(&creadfds);
         FD_ZERO(&cwritefds);
         int cfdmax = MAX(cmd_sd, data_sd);
         /* Il veille à ce que les sockets soient non bloquant*/
         fcntl(cmd_sd, F_SETFL, O_NONBLOCK );
         fcntl(data_sd, F_SETFL, O_NONBLOCK );
         while(1){
           /* Configuration des descripteurs de fichier pour select */
           FD_SET(data_sd, &creadfds);
           FD_SET(data_sd, &cwritefds);
           FD_SET(cmd_sd, &creadfds);
           FD_SET(cmd_sd, &cwritefds);
           ret = select(cfdmax+1, &creadfds, &cwritefds,NULL,NULL);
           /* Le serveur vient de recevoir une commande d'un client */
           if(FD_ISSET(cmd_sd, &creadfds)){
             msg_receive(cmd_sd, &m_in);
             data_fd = handle_msg(&m_in, &m_out, data_sd);
             should_send_cmd = 1;
             if(data_fd > 0)
               should_send_data = 1;
             FD_CLR(cmd_sd, &creadfds);
           }
           /*Le serveur s'apprête à envoyer une commande au client */
           if(FD_ISSET(cmd_sd, &cwritefds)){
             if(should_send_cmd){
               /* TODO vérifier que m_out est un message valide */
               msg_send(cmd_sd, &m_out);
               should_send_cmd = 0;
             }
             FD_CLR(cmd_sd, &cwritefds);
           }
           /* le serveur a reçu des donnée du client */
           if(FD_ISSET(data_sd, &creadfds)){
             /* TODO */ 
             FD_CLR(data_sd, &creadfds);
           }
           /* le serveur a des données à envoyer*/
           if(FD_ISSET(data_sd, &cwritefds)){
             /* l'appel système send() avait entre temps échoué, il faut renvoyer
              * les données qui ont échouées et qui sont dans le buffer d'envoie
              */
             if(failed_send && send_length){
                ds = send(data_sd, buffer+offset, send_length,0);
                if(ds < 0){
                   failed_send = 1;
               }
               else{
                 if((ds == send_length) || !send_length ){
                   failed_send =  0;
                   send_length = 0;
                   offset = 0;
                 }else{
                   send_length-=ds;
                   offset+=ds;
                 }
               }
             }
             
             /* Toutes les données ont été envoyées on peut lire des données 
              * du fichier si on est pas à la fin du fichier 
              */
             if(should_send_data && (data_fd > 0) && !failed_send && !send_length){
               while((ret = read(data_fd, buffer, BUF_SIZE)) > 0){
                 data_read += ret;
                 ds = send(data_sd, buffer, ret,0);
                 /** Il n'y a plus d'espace dans le buffer d'envoie de TCP **/
                 if((ds < 0) || (ds < ret)){
                   failed_send = 1;
                   send_length = (ds < 0) ? ret : ret - ds;
                   if(ds < ret){
                     data_sent += ds;
                     offset = ds;
                   }else offset = 0;
                   break;
                 }
                 else{
                   /* toutes les données du buffer d'envoie ont pu être envoyées*/
                   data_sent += ds;
                   failed_send = 0;
                 }
               }
               if(ret == 0){
                 /* Etant à la fin du fichier il faut fermer le fichier*/
                 close(data_fd);
                 should_send_data = 0;
                 data_fd = -1;
               }
               if(ret < 0){
                 /** TODO il faut gérer l'erreur**/
               }
             }
             FD_CLR(data_sd, &cwritefds);
           }
         }
       }
    }
  }
/******************************************************************************/
  return 0;
}

static int tcp_listen(char *server_ipaddr, char *server_port){
  int sd = 0;
  struct sockaddr server_addr;
  socklen_t salen;
  sd = socket(AF_INET, SOCK_STREAM,0);
  if(sd == -1){
    perror("socket échec !!");
    exit(1);
  }
  if(resolve_address(&server_addr, &salen, server_ipaddr, server_port, 
      AF_INET, SOCK_STREAM, IPPROTO_TCP)!= 0){
      fprintf(stderr, "Erreur de configuration de sockaddr\n");
      return -1;
  }
  if(bind(sd, &server_addr, salen) < 0){
    perror("Echec de liaison !!");
    return -1;
  }
  listen(sd,BACKLOG_SIZE);
  return sd; 
}



int tcp_ls(int sock_msg, int sock_file){
	int value;
	int filehandle;
	int size;
	struct stat obj;
	system("ls > ls.txt");
	stat("ls.txt",&obj);
	size = obj.st_size;
	send(sock_msg,&size,sizeof(int),0);
	filehandle = open("ls.txt",O_RDONLY);
	value = sendfile(sock_file,filehandle,NULL,size);
	return value; 
}


int tcp_get(int sock_msg, int sock_file){
	char filename[50];
	struct stat obj;
	int filehandle;
	//char buffer[2048];
	int size;

	recv(sock_msg,filename,50,0);
	printf("Nom du fichier : %s\n",filename);
	stat(filename,&obj);
	filehandle = open(filename, O_RDONLY);
	size = obj.st_size;

	if(filehandle == -1){
		size = 0;
		printf("Taille 0\n");
	}

	//S'il existe un fichier, la taille du fichier est envoyée.
	send(sock_msg, &size, sizeof(int), 0);

	printf("Taille du fichier : %d octets\n",size);
	if(size){
		int result = 0;
		result = sendfile(sock_file,filehandle, NULL, size);
		printf("Envoi du fichier terminé %d!!\n", result);
	}
	close(filehandle);
	return 0;
}





int tcp_put(int sock_msg, int sock_file){
	int result = 0;
	//int c = 0;
	//int len;
	char *f;
	int size;
	int filehandle;
	char filename[50];
	sleep(5);
	//Reception du nom du fichier à télécharger
	recv(sock_msg, filename, 100,0);
	//Nom de fichier
	printf("Nom du fichier : %s\n",filename);

	//Reception la taille du fichier
	recv(sock_msg, &size, sizeof(int),0);
	if(!size){
		printf("Il n'y a pas de fichier\n");
		return -1;
	}
	f = malloc(size);
	//Recevoir le fichier
	recv(sock_file, f, size,0);
	//Enregistrement de fichier
	while(1)
	{
		filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
		if(filehandle == -1){
			sprintf(filename + strlen(filename), "_1");
		}
		else{
			break;
		}
	}
	write(filehandle, f, size);
	close(filehandle);
	return result; 
}


//FTP Server

int tcp_pwd(int sock_msg, int sock_file){
	int value;
	int filehandle;
	struct stat obj;
	int size;

	system("pwd>pwd.txt");
	stat("pwd.txt",&obj);
	size = obj.st_size;

	value = send(sock_msg,&size,sizeof(int),0);
	filehandle = open("pwd.txt",O_RDONLY);
	sendfile(sock_file,filehandle,NULL,size);

	return value; 
}


int tcp_cd(int sock_msg){
	int value;
	int c;
	char buf_value[100];

	sleep(5);
	recv(sock_msg,buf_value,100,0);
	if(chdir(buf_value + 3) == 0){
		c = 1;
	}
	else{
		c = 0;
	}
	value = send(sock_msg,&c,sizeof(int),0);
	return value;
}


void exitClient(){
	num_user--;
	exit(0); 
}


void main_select(int sock_msg, int sock_file){
	char buf[100];
	int result = 0;
	char result1[15];
	char client_char[100];
	char msg[50];
	sprintf(result1,"%d",num_user);

	strcpy(msg,"Numéro_client ");
	strcat(msg,result1);
	strcat(msg, " Connect!! , ");
	filehandle = open("Story.txt", O_CREAT | O_APPEND | O_RDWR, 0666);
	write(filehandle, msg, strlen(msg));
	close(filehandle);
	while(1){
		recv(sock_msg,buf,100,0);
		recv(sock_msg, buf, 100, 0);

		if(!strcmp(buf,"ls")){
			//ls
			result = tcp_ls(sock_msg, sock_file);
			if(result==-1){
				printf("ls\n");
			}
			else{
				printf("Commande : ls\n");
			}
		}
		else if(!strcmp(buf,"get")){
			//get
			result = tcp_get(sock_msg, sock_file);

			if(result==-1){
				printf("get\n");
			}
			else{
				printf("Commande : get\n");
			}
		}
		else if(!strcmp(buf,"put")){
			//put
			result = tcp_put(sock_msg, sock_file);
			if(result==-1){
				printf("put\n");
			}
			else{
				printf("Commande : put\n");
			}
		}
		else if(!strcmp(buf,"pwd")){
			//pwd
			result = tcp_pwd(sock_msg, sock_file);
			if(result==-1){
				printf("pwd !!\n");
			}else{
				printf("Commande : pwd \n");
			}
		}
		else if(!strcmp(buf,"cd")){
			//cd
			result = tcp_cd(sock_msg);

			if(result == -1){
				printf("cd\n");
			}else{
				printf("Commande : cd\n");
			}
		}
		else if(!strcmp(buf,"quit")){
			//quit
			exitClient();
		}
		strcpy(client_char,"Client_Number");
		strcat(client_char,result1);
		strcat(client_char, " : ");
		strcat(client_char,buf);

		if(sock_msg==-1){
			strcat(client_char," Echec!! , ");
		}
		else{
			strcat(client_char," success , ");
		}

		filehandle = open("Story.txt", O_CREAT | O_APPEND | O_RDWR, 0666);
		write(filehandle, client_char, strlen(client_char));
		close(filehandle);
	} 
}

