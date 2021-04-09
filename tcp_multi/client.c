#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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


int main(int argc, char *argv[]){
	struct sockaddr_in server;
	struct stat obj;
	int sock,sock1;
	char bufmsg[MAXLINE];
	int port;
	char ip[MAXLINE];
	int result;
	int pid_number;
	char msg[MAXLINE];
	while(1){
		printf(" Entrer le port du Server : ");
		scanf("%d",&port);

		TCP_Connect(AF_INET, bufmsg, port, &sock);
		TCP_Connect(AF_INET, bufmsg, port+1, &sock1);
		if(sock == -1 && sock1 == -1){
			//printf("FTP !!\n");
			exit(1);
		}
		else{
			recv(sock,msg,100,0);
			recv(sock,&pid_number,100,0);
			printf("%s\n",&msg);
			printf("Numéro du PID : %d\n",pid_number);
			//printf("%d - %d\n",sock, sock1);
			while(1){
				//printf("Les commandes get, put, pwd, ls, cd, quit, client_cd, client_pwd, client_ls, help\n");
				Message();
				Screen_print();
				scanf("%s",bufmsg);

				if((send(sock,bufmsg,100,0)) < 0)
				{
					perror("send: ");
				}
				if(!strcmp(bufmsg,"get")){
					printf("Commande get \n");
					result = File_get(sock, sock1);
					if(result==-1){
						printf("get La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("get J'ai exécuté la commande normalement\n");
					}
				}
				else if(!strcmp(bufmsg,"put")){
					printf("Commande put \n");
					result = File_put(sock, sock1);
					if(result==-1){
						printf("put La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("put J'ai exécuté la commande normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"pwd")){
					printf("Commande pwd\n");
					result = File_pwd(sock, sock1);
					if(result==-1){
						printf("pwd La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("pwd La commande a été exécutée normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"ls")){
					printf("Commande ls \n");
					result = File_ls(sock, sock1);
					if(result==-1){
						printf("ls La commande n'a pas pu être exécutée normalement.\n");
					}
					else{
						//printf("ls La commande a été exécutée normalement.\n");
					}
				}
				else if(!strcmp(bufmsg,"cd")){
					printf("Commande cd\n");
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
    printf("\n\t\t        =   -get      -put    	-pwd       -ls      =");
    printf("\n\t\t        =   -cd         -quit     -client_cd        =");
    printf("\n\t\t        =   -client_pwd      -client_ls    -aide    =");
    printf("\n\t\t        =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf("\n\t\t  **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**\n");
    printf("\n");
}


void TCP_Connect(int af, char *servip, unsigned int port, int *sock)
{
	struct sockaddr_in servaddr;
	if ((*sock = socket(af, SOCK_STREAM, 0)) < 0){
		return -1;
	}
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if(connect(*sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		return -1;
	}
	//return s; 
}



int File_put(int sock_msg,int sock_file){
	char filename[MAXLINE];
	int filehandle;
	char buf[20];
	struct stat obj;
	int size;
	int value;
	int status;

	send(sock_msg, "put", 100,0);
	printf(" Saisissez le fichier à télécharger. \n");
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
	char buf[50];
	char filename[MAXLINE];
	char temp[MAXLINE];
	int size;
	int value;
	char *f;
	int filehandle;

	send(sock_msg, "get", 100,0);
	printf("Veuillez saisir le fichier à télécharger \n");
	Screen_print();
	scanf("%s",filename);
	printf("%s\n",filename);
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
	printf("size value : %d\n",size);
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
	printf(" Téléchargement complet !!\n");
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
	printf("send value : %d\n",value);
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
	printf("size : %d\n",size);
	printf("value : %d\n",value);
	filehandle = open("ls.txt",O_RDWR | O_CREAT ,0666);
	write(filehandle, f, size);
	close(filehandle);
	printf(" Liste des répertoires \n");
	system("cat ls.txt");

	return value; 
}



int File_cd(int sock_msg){
	char buf[100];
	char temp[20];
	int value;
	int status;

	send(sock_msg,"cd",100,0);
	strcpy(buf,"cd ");
	printf(" Entrez le chemin à déplacer \n"); 
	Screen_print();
	scanf("%s", temp);
	strcat(buf,temp);
	printf("%s\n",buf);
	send(sock_msg,buf,100,0);
	recv(sock_msg, &status, sizeof(int), 0);

	printf("%d\n",status);
	if (status){
		printf(" Changement de chemin complet \n");
	}
	else{
		printf(" Le changement de chemin a échoué \n");
	}
	return value; 
}



void Quit(int sock_msg){
	// int status;
	// char buf[100];
	// strcpy(buf,"quit");
	// send(sock_msg,buf,100,0);
	// status = recv(sock_msg,&status,100,0);
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
	printf("Entrez le nom du chemin vers lequel vous souhaitez accéder\n");
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
	printf("cmd>");
}
	

int Client_ls(){
	int result;
	printf("Liste des fichiers \n");
	result = system("ls");
	return result;
}



void Help(){
	printf ("Usage:: EX: get filename.extension");
	printf ("get télécharge un fichier du serveur vers le client. \n");
	printf ("put télécharge un fichier du client vers le serveur. \n");
	printf ("pwd montre le chemin vers le serveur. \n");
	printf ("ls affiche une liste de fichiers sur le serveur. \n");
	printf ("cd déplace le chemin vers le serveur. \n");
	printf ("quit arrête le serveur. \n");
	printf ("client_cd déplace le chemin du client. \n");
	printf ("client_pwd montre le chemin du client. \n");
	printf ("client_ls affiche une liste de fichiers sur le client. \n");
}