#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


#define MAXLINE 200

void TCP_Connect(int af, char *servip, unsigned int port, int *sock);
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
  int sock,sock1;
  char bufmsg[MAXLINE];
  int port1, port2;
  int result;
  int pid_number;
  char msg[MAXLINE];
  char c;
  while((c = getopt(argc, argv, "p:P:")) != -1) {
    switch(c){
      case 'p':
        port1 = atoi(optarg);
        break;
      case 'P':
        port2 = atoi(optarg);
        break;
      default:
        break;
      
    }
  }
  while(1){
    TCP_Connect(AF_INET, bufmsg, port1, &sock);
    TCP_Connect(AF_INET, bufmsg, port2, &sock1);
    if(sock == -1 && sock1 == -1)
      exit(1);
    else{
      recv(sock,msg,100,0);
      recv(sock,&pid_number,100,0);
      printf("%s\n",msg);
      printf("Numéro du PID : %d\n",pid_number);
      //printf("%d - %d\n",sock, sock1);
      while(1){
        Message();
        Screen_print();
        scanf("%s",bufmsg);
        if((send(sock,bufmsg,100,0)) < 0)
          perror("send: ");
        if(!strcmp(bufmsg,"get")){
          result = File_get(sock, sock1);
          if(result==-1){
            printf("get La commande n'a pas pu être exécutée normalement.\n");
          }
	}
	else if(!strcmp(bufmsg,"put")){
	  result = File_put(sock, sock1);
	  if(result==-1){
	    printf("put La commande n'a pas pu être exécutée normalement.\n");
	  }
	  else{
	    //printf("put J'ai exécuté la commande normalement.\n");
	  }
	}
	else if(!strcmp(bufmsg,"pwd")){
	  result = File_pwd(sock, sock1);
	  if(result==-1){
	    printf("pwd La commande n'a pas pu être exécutée normalement.\n");
	  }
	  else{
						//printf("pwd La commande a été exécutée normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"ls")){
					result = File_ls(sock, sock1);
					if(result==-1){
						printf("ls La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("ls La commande a été exécutée normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"cd")){
					result = File_cd(sock);
					if(result==-1){
						printf("cd La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("cd J'ai exécuté la commande normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"quit")){
					Quit(sock);
					if(result==-1){
						printf("Échec de l'arrêt du server.\n");
					}
					else{
						printf("Arrêt du serveur.\n");
						exit(1);
					}
				}
				else if(!strcmp(bufmsg,"client_cd")){
					result = Client_cd();
					if(result==-1){
						printf("client_cd La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("client_cd J'ai exécuté la commande normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"client_pwd")){
					result = Client_pwd();
					if(result==-1){
						printf("client_pwd La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("client_pwd J'ai exécuté la commande normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"client_ls")){
					result = Client_ls();
					if(result==-1){
						printf("client_ls La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("client_ls J'ai exécuté la commande normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"aide")){
					Help();
				}
				else{
					printf("Veuillez saisir à nouveau\n");
				}
			}
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


void TCP_Connect(int af, char *servip, unsigned int port, int *sock)
{
	struct sockaddr_in servaddr;
	if ((*sock = socket(af, SOCK_STREAM, 0)) < 0){
		//return -1;
	}
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if(connect(*sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		//return -1;
	}
	//return s; 
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
