#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAXLINE  511

void headMessage(void)
{
    printf("\t\t\t#######################################");
    printf("\n\t\t\t#####                            ######");
    printf("\n\t\t\t#####   WELCOME TO TCP  Client   ######");
    printf("\n\t\t\t#####                            ######");
    printf("\n\t\t\t#######################################");
}

int tcp_connect(int af, char *servip, unsigned short port) 
{
	struct sockaddr_in servaddr;
	int  s;
	
	if ((s = socket(af, SOCK_STREAM, 0)) < 0)
		return -1;
	
	printf("\n Création du socket ... \n");
	
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);

	
	if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		return -1;

	printf(" Connexion au serveur établie ... \n En attente de commande ");
	return s;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct stat obj;
	int sock;
	char bufmsg[MAXLINE];
	char buf[100], command[5], filename[MAXLINE], *f;
	char temp[20];
	int k, size, status;
	int filehandle;

	if (argc != 3) {
		printf("Usage : %s <server_ip> <port> \n", argv[0]);
		exit(1);
	}

	sock = tcp_connect(AF_INET, argv[1], atoi(argv[2]));
	if (sock == -1) 
    {
		printf("Connexion TCP échouée");
		exit(1);
	}

	while (1)  
    {
		printf("\033[1;33m \n Les commandes : get, put, pwd, ls, cd, quit\n");
		printf("\033[1;32m client> ");
		fgets(bufmsg, MAXLINE, stdin); 
		fprintf(stderr, "\033[97m");   
		if (!strcmp(bufmsg, "get\n")) 
        {
			printf("Fichier à télécharger : ");
			scanf("%s", filename);       
			fgets(temp, MAXLINE, stdin); 
			strcpy(buf, "get ");
			strcat(buf, filename);
			send(sock, buf, 100, 0);         
			recv(sock, &size, sizeof(int), 0);
			if (!size) 
            {
				printf("Aucun fichier\n");
				continue;
			}
			f = malloc(size);
			recv(sock, f, size, 0);
			while (1) {
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
				if (filehandle == -1) 
					sprintf(filename + strlen(filename), "_1");
				else 
                    break;
			}
			write(filehandle, f, size, 0);
			close(filehandle);
			printf("Téléchargement complet\n");
		}
		else if (!strcmp(bufmsg, "put\n")) 
        {
			printf("Fichier à télécharger : ");
			scanf("%s", filename);       
			fgets(temp, MAXLINE, stdin); 
			filehandle = open(filename, O_RDONLY);
			if (filehandle == -1) 
            {
				printf("Il n'y a pas de fichier.\n");
				continue;
			}
			strcpy(buf, "put ");
			strcat(buf, filename);
			send(sock, buf, 100, 0);
			stat(filename, &obj);
			size = obj.st_size;
			send(sock, &size, sizeof(int), 0);
			sendfile(sock, filehandle, NULL, size);
			recv(sock, &status, sizeof(int), 0);
			if (status)
				printf("Téléchargement complet\n");
			else
				printf("Échec du téléchargement\n");
		}
		else if (!strcmp(bufmsg, "pwd\n")) 
        {
			strcpy(buf, "pwd");
			send(sock, buf, 100, 0);
			recv(sock, buf, 100, 0);
			printf("Le chemin du répertoire distant \n %s", buf);
		}
		else if (!strcmp(bufmsg, "ls\n")) 
		{
			strcpy(buf, "ls");
			send(sock, buf, 100, 0);
			recv(sock, &size, sizeof(int), 0);
			f = malloc(size);
			recv(sock, f, size, 0);
			filehandle = creat("temps.txt", O_WRONLY);
			write(filehandle, f, size, 0);
			close(filehandle);
			printf("La liste du contenu du répertoire distant\n");
			system("cat temps.txt");	
		}
		else if (!strcmp(bufmsg, "cd\n")) 
        {
			strcpy(buf, "cd ");
			printf("Nom du chemin d'accès : ");
			scanf("%s", buf + 3);        
			fgets(temp, MAXLINE, stdin); 
			send(sock, buf, 100, 0);     
			recv(sock, &status, sizeof(int), 0);
			if (status)
				printf("Changement de répertoire terminé\n");
			else
				printf("Échec du changement de chemin\n");
		}
		else if (!strcmp(bufmsg, "quit\n")) 
        {
			strcpy(buf, "quit");
			send(sock, buf, 100, 0);
			recv(sock, &status, 100, 0);
			if (status) {
				printf("Fermeture du serveur..\n");
				exit(0);
			}
			printf("Échec de fermeture du serveur\n");
		}
	}
}