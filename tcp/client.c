#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
//#include <unistd.h>


#define MAXLINE 200

int TCP_Connexion(int af, char *servip, unsigned int port);
int File_put(int sock_msg, int sock_file);
int File_get(int sock_msg, int sock_file);
int File_ls(int sock_msg, int sock_file);
int File_pwd(int sock_msg, int sock_file);
int File_cd(int sock_msg);
int File_quit(int sock_msg);
void CMD_help();
int Client_cd();
int Client_pwd();
int Client_ls();
void Screen_print();


int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct stat obj;
	int sock,sock1;
	char bufmsg[MAXLINE];
	int port;
	char ip[MAXLINE];
	int result;

	while(1)
	{
		printf("Entrer le port du Server : ");
		scanf("%d",&port);
		sock = TCP_Connexion(AF_INET, bufmsg, port);
		sock1 = TCP_Connexion(AF_INET,bufmsg, port+1);
		if(sock == -1)
		{
			printf("Connexion FTP échouée !!\n");
			exit(1);
		}
		else{
			printf("Connecion FTP réussie !!\n");
			while(1)
			{
				Message();
				Screen_print();
				scanf("%s",bufmsg);
				if(!strcmp(bufmsg,"get"))
				{
					printf("Commande get entrée\n");
					result = File_get(sock, sock1);
					if(result==-1){
						printf("Commande get échec.\n");
					}
					else{
						//printf("Commande get succès\n");
					}
				}
				else if(!strcmp(bufmsg,"put"))
				{
					printf("Commande put\n");
					result = File_put(sock, sock1);
					if(result==-1){
						printf("Commande put échec.\n");
					}
					else{
						//printf("Commande put succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"pwd"))
				{
					printf("Commande pwd \n");
					result = File_pwd(sock,sock1);
					if(result==-1){
						printf("Commande pwd échec.\n");
					}
					else{
						//printf("Commande pwd succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"ls"))
				{
					printf("Commande ls \n");
					result = File_ls(sock, sock1);
					if(result==-1){
						printf("Commande ls échec.\n");
					}
					else{
						//printf("Commande ls succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"cd"))
				{
					printf("Commande cd \n");
					result = File_cd(sock);
					if(result==-1){
						printf("Commande cd échec.\n");
					}
					else{
						//printf("Commande cd succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"quit"))
				{
					result = File_quit(sock);
					if(result==-1){
						printf("Arrêt du server impossible.\n");
					}
					else{
						printf("Arrêt du server.\n");
						exit(1);
					}
				}
				else if(!strcmp(bufmsg,"client_cd")){
					result = Client_cd();
					if(result==-1){
						printf("Commande client_cd échec.\n");
					}
					else{
						//printf("Commande client_cd succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"client_pwd"))
				{
					result = Client_pwd();

					if(result==-1){
						printf("Commande client_pwd échec.\n");
					}
					else{
						//printf("Commande client_pwd succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"client_ls"))
				{
					result = Client_ls();
					if(result==-1){
						printf("Commande client_ls échec.\n");
					}
					else{
						//printf("Commande client_ls succès.\n");
					}
				}
				else if(!strcmp(bufmsg,"Aide")){
					CMD_help();
				}
				else{
					printf("Veuillez réessayer\n");
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
    printf("\n\t\t        =   -get      -put    	-pwd       -ls    =");
    printf("\n\t\t        =   -cd         -quit     -client_cd        =");
    printf("\n\t\t        =   -client_pwd      -client_ls    -aide    =");
    printf("\n\t\t        =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf("\n\t\t  **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**\n");
    printf("\n");
}


int TCP_Connexion(int af, char *servip, unsigned int port)
{
	struct sockaddr_in servaddr;
	int s;
	if ((s = socket(af, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if(connect(s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		return -1;
	}
	return s;
}



int File_put(int sock_msg, int sock_file)
{
	char filename[MAXLINE];
	char temp[20];
	int filehandle;
	char buf[20];
	struct stat obj;
	int size;
 	int value;
 	int status;

	printf("Entrez un fichier à télécharger\n");
	Screen_print();
	scanf("%s",filename);
	fgets(temp,MAXLINE,stdin);
	filehandle = open(filename, O_RDONLY);
	if(filehandle == -1)
	{
		printf("Le fichier n'existe pas.\n");
		return -1;
	}
	strcpy(buf,"put ");
	strcat(buf, filename);
	send(sock_msg,buf,100,0);
	stat(filename,&obj);
	size = obj.st_size;
	send(sock_msg, &size, sizeof(int), 0);
	sendfile(sock_file, filehandle, NULL, size);
	value = recv(sock_msg, &status, sizeof(int), 0);
	if(status){
		printf("Téléchargement terminé \n");
	}
	else{
		printf("Echec de téléchargement\n");
	}
	return value; 
}



int File_get(int sock_msg, int sock_file)
{
	char buf[50];
	char filename[MAXLINE];
	char temp[MAXLINE];
	int size;
	int value;
	char *f;
	int filehandle;

	printf("Veuillez saisir un fichier à télécharger\n");
	Screen_print();
	scanf("%s",filename);

	fgets(temp,MAXLINE,stdin);
	strcpy(buf,"get ");
	strcat(buf, filename);
	send(sock_msg, buf, 100, 0);
	recv(sock_msg, &size, sizeof(int), 0);
	if(!size){
		printf("Aucun fichier\n");
		return -1;
	}
	f = malloc(size);
	recv(sock_file,f,size,0);
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
	write(filehandle, f, size, 0);
	close(filehandle);
	printf("Téléchargement terminé !!\n");
	return value; 
}



int File_pwd(int sock_msg, int sock_file)
{
	int value;
	char buf[100];
	int size;
	int filehandle;
	char *f;

	strcpy(buf,"pwd");
	send(sock_msg,buf,100,0);
	value = recv(sock_msg, &size, sizeof(int), 0);
	f = malloc(size);
	recv(sock_file,f,size,0);
	filehandle = open("pwd.txt",O_RDWR | O_CREAT,0666);
	write(filehandle,f,size,0);
	close(filehandle);
	printf("Résultat de la commande pwd: %s\n",buf);
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
	send(sock_msg, buf, 100,0);
	value = recv(sock_msg,&size,sizeof(int),0);
	f = malloc(size);
	recv(sock_file,f,size,0);
	filehandle = open("ls.txt",O_RDWR | O_CREAT ,0666);
	write(filehandle, f, size, 0);
	close(filehandle);
	printf("liste des répertoires\n");
	system("cat ls.txt");
	return value; 
}



int File_cd(int sock_msg)
{
	char buf[100];
	char temp[20];
	int value;
	int status;

	strcpy(buf,"cd ");
	printf("Entrez le chemin d'accès\n");
	Screen_print();
	scanf("%s", buf + 3);
	fgets(temp, MAXLINE, stdin);
	send(sock_msg,buf,100,0);
	recv(sock_msg, &status, sizeof(int), 0);
	printf("%d\n",status);
	if (status){
 		printf("Changement de chemin terminé\n");
	}
	else{
		printf("Impossible de changer de chemin \n");
	}
	return value; 
}


int File_quit(int sock_msg){
	int status;
	char buf[100];
	strcpy(buf,"quit");
	send(sock_msg,buf,100,0);
	status = recv(sock_msg,&status,100,0);
	return status; 
}


int Client_pwd(){
	int result;
	result = system("pwd");
	return result; 
}


int Client_cd()
{
	int result;
	char input[100];

	strcpy(input,"cd ");
	printf("Entrez le nom du chemin vers \n");
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
	printf("ftp>"); 
}


int Client_ls(){
	int result;
	printf("liste des fichiers  \n");
	result = system("ls");
	return result;
}



void CMD_help(){
	printf("Comment utiliser : Ex get file_name.extension");
	printf("get Télécharge un fichier du server vers le client.\n");
	printf("put Télécharge un fichier du client vers le server.\n");
	printf("pwd montre le chemin vers le server.\n");
	printf("ls affiche la liste des fichiers sur le server.\n");
	printf("cd déplace le chemin vers le server.\n");
	printf("quit Arrête le server.\n");
	printf("client_cd Déplace le chemin du client.\n");
	printf("client_pwd montre le chemin du client.\n");
	printf("client_lsAffiche la liste des des fichiers sur le client.\n");
}