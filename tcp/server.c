#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>
//#include <unistd.h>


int tcp_listen(int host, int port, int backlog);
int tcp_ls(int sock_msg, int sock_file);
int tcp_get(int sock_msg,char *buf_value, int sock_file);
int tcp_put(int sock_msg, char *buf_value, char *comm_value, int sock_file);
int tcp_pwd(int sock_msg, int sock_file);
int tcp_cd(int sock_msg, char *buf_value);
int tcp_quit(int sock_msg);

int main(int argc, char *argv[])
{
	//Structure contenant l'adresse du socket (IP + Port Number, Famille d'adresse)
	struct sockaddr_in server, client;
	char buf[100], command[5], filename[20];
	int sock1, sock2, sock3, sock4;
	int input, result, filehandle, len;
	

	printf("Entrer un Numéro de Port : ");
	scanf("%d", &input);

	sock1 = tcp_listen(INADDR_ANY, input, 5);
	sock2 = tcp_listen(INADDR_ANY, input+1, 6);
	len = sizeof(client);
	sock3 = accept(sock1, (struct sockaddr*)&client, &len);
	sock4 = accept(sock2, (struct sockaddr*)&client, &len);

	while(1)
	{
		//Reception de la commande du client 
		recv(sock3, buf, 100, 0);
		sscanf(buf,"%s",command);
		if(!strcmp(command,"ls"))
		{
			result = tcp_ls(sock3, sock4);
			if(result==-1){
				printf("La commande <ls> a échouée !!\n");
			}
			else{
				printf("Commande <ls> réussie !!\n");
			}
		}
		else if(!strcmp(command,"get"))
		{
			result = tcp_get(sock3,buf,sock4);
			if(result==-1){
				printf(" Erreur de téléchargement du fichier!!\n");
			}
			else{
				printf("Téléchargement du fichier réussi !!\n");
			}
		}
		else if(!strcmp(command,"put"))
		{
			result = tcp_put(sock3,buf,command,sock4);
			if(result==-1){
				printf("Erreur de téléchargement du fichier!!\n");
			}
			else{
				printf("Téléchargement du fichier réussi!!\n");
			}
		}
		else if(!strcmp(command,"pwd"))
		{
			result = tcp_pwd(sock3, sock4);
			if(result==-1){
				printf("Impossible d'exécuter la commande pwd!!\n");
			}
			else{
				printf("Exécution la commande pwd réussie !!\n");
			}
		}
		else if(!strcmp(command,"cd"))
		{
			result = tcp_cd(sock3, buf);

			if(result == -1){
				printf("Impossible d'exécuter la commande cd !!\n");
			}
			else{
				printf("Exécution la commande cd réussie  !!\n");
			}
		}
		else if(!strcmp(command,"quit"))
		{
			//quit
			result = tcp_quit(sock3);
			if(result ==-1){
				printf("L'arret du Server a échouée !!\n");
			}
			else{
				printf("Server terminé avec succès !!\n");
				exit(0);
			}
		}
	}
	return 0; 
}

int tcp_listen(int host, int port, int backlog)
{
	int sd;
	struct sockaddr_in servaddr;

	sd = socket(AF_INET, SOCK_STREAM,0);
	if(sd == -1){
		perror("socket échec!!");
		exit(1);
	}
	//bzero
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	//IP Address
	servaddr.sin_addr.s_addr = htonl(host);
	//Port
	servaddr.sin_port = htons(port);
	//IP
	if(bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Echec de liaison !!");
		exit(1);
	}
	//
	listen(sd,backlog);
	return sd; 
}


//ls
int tcp_ls(int sock_msg, int sock_file)
{
	int value;
	int filehandle;
	int size;
	struct stat obj;
	system("ls > ls.txt");
	stat("ls.txt",&obj);
	size = obj.st_size;
	send(sock_msg, &size, sizeof(int), 0);
	filehandle = open("ls.txt",O_RDONLY);
	value = sendfile(sock_file,filehandle,NULL,size);
	return value; 
}




int tcp_get(int sock_msg, char *buf_value, int sock_file)
{
	char filename[20];
	struct stat obj;
	int filehandle;
	int result;
	int size;
	sscanf(buf_value,"%s%s",filename,filename);
	stat(filename,&obj);

	filehandle = open(filename, O_RDONLY);
	size = obj.st_size;
	if(filehandle == -1){
		size = 0;
	}

	result = send(sock_msg, &size, sizeof(int), 0);

	if(size){
		result = sendfile(sock_file,filehandle, NULL, size);
	}
	return result; 
}



int tcp_put(int sock_msg, char *buf_value, char *comm_value, int sock_file)
{
	int result;
	int c = 0;
	int len;
	char *f;
	int size;
	int filehandle;
	int filename[50];

	sscanf(buf_value + strlen(comm_value), "%s", filename);
	recv(sock_msg, &size, sizeof(int),0);

	while (1)
	{
		filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
		if(filehandle == -1){
			sprintf(filename + strlen(filename), "_1");
		}
		else{
			break;
		}
	}

	f = malloc(size);
	recv(sock_file, f, size,0);
	c = write(filehandle, f, size);
	close(filehandle);
	result = send(sock_msg, &c, sizeof(int), 0);
	return result; 
}



//FTP Server
int tcp_pwd(int sock_msg, int sock_file)
{
	int value;
	int filehandle;
	struct stat obj;
	int size;

	system("pwd>pwd.txt");
	stat("pwd.txt", &obj);
	size = obj.st_size;
	value = send(sock_msg, &size, sizeof(int), 0);
	filehandle = open("pwd.txt", O_RDONLY);
	sendfile(sock_file, filehandle, NULL, size);
	return value; 
}




int tcp_cd(int sock_msg, char *buf_value)
{
	int value;
	int c;

	printf("%s", buf_value);
	if(chdir(buf_value + 3) == 0){
		c = 1;
	}
	else{
		c = 0;
	}
	value = send(sock_msg, &c, sizeof(int), 0);
	return value; 
}



//Sever
int tcp_quit(int sock_msg)
{
	int value;
	int i;
	value = send(sock_msg, &i, sizeof(int), 0);
	return value; 
}