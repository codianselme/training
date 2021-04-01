#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>

void welcomeMessage(void)
{
    printf("\n");
    printf("\n\t\t  **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**");
    printf("\n\t\t        =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf("\n\t\t        =                 WELCOME TO TCP            =");
    printf("\n\t\t        =                     SERVER                =");
    printf("\n\t\t        =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    printf("\n\t\t  **-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**-**\n");
    printf("\n\t Enter any key to continue..... \n");
    getchar();
}

int tcp_init(int host, int port, int backlog) 
{
	int sd;
	struct sockaddr_in servaddr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("Echec de création du socket");
		exit(1);
	}

	printf("Création du socket ... \n");
    sleep(1);
	
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    {
		perror("Echec de la liaison");  
        exit(1);
	}
	
	printf("Liaison établie ... \nAdresse : %d \nPort : %i \n", htonl(host), htons(port));
    sleep(1);

	listen(sd, backlog);

	printf("Serveur en attente de connexion client ... \n");
    sleep(1);
	
	return sd;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server, client;
	struct stat obj;
	int sock1, sock2;
	char buf[100], command[5], filename[20];
	int k, i, size, len, c;
	int filehandle;

	sock1 = tcp_init(INADDR_ANY, atoi(argv[1]), 5);

	len = sizeof(client);
	sock2 = accept(sock1, (struct sockaddr*)&client, &len);
	printf("\nConnexion etablie ...");

	while (1) 
	{
		recv(sock2, buf, 100, 0);
		sscanf(buf, "%s", command);

		if (!strcmp(command, "ls")) 
        {
			system("ls >temps.txt");
			stat("temps.txt", &obj);
			size = obj.st_size;
			send(sock2, &size, sizeof(int), 0);
			filehandle = open("temps.txt", O_RDONLY);
			sendfile(sock2, filehandle, NULL, size);
		}
		else if (!strcmp(command, "get")) 
        {
			sscanf(buf, "%s%s", filename, filename);
			stat(filename, &obj);
			filehandle = open(filename, O_RDONLY);
			size = obj.st_size;
			if (filehandle == -1)
				size = 0;
			send(sock2, &size, sizeof(int), 0);
			if (size)
				sendfile(sock2, filehandle, NULL, size);
		}
		else if (!strcmp(command, "put")) 
        {
			int c = 0, len;
			char *f;
			sscanf(buf + strlen(command), "%s", filename);
			recv(sock2, &size, sizeof(int), 0);

			while (1) 
            {
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
				if (filehandle == -1)
					sprintf(filename + strlen(filename), "_1");
				else break;
			}
			f = malloc(size);
			recv(sock2, f, size, 0);
			c = write(filehandle, f, size);
			close(filehandle);
			send(sock2, &c, sizeof(int), 0);
		}
		else if (!strcmp(command, "pwd")) 
        {
			system("pwd>temp.txt");
			i = 0;
			FILE*f = fopen("temp.txt", "r");
			while (!feof(f)) buf[i++] = fgetc(f);
			buf[i - 1] = '\0';
			fclose(f);
			send(sock2, buf, 100, 0);
		} 
		else if (!strcmp(command, "cd")) 
        {
			if (chdir(buf + 3) == 0) c = 1;
			else c = 0;	
			send(sock2, &c, sizeof(int), 0);
		}
		else if (!strcmp(command, "Au revoir") || !strcmp(command, "quit")) 
        {
			printf("Fermeture du serveur..\n");
			i = 1;
			send(sock2, &i, sizeof(int), 0);
			exit(0);
		}
	}
	return 0;
}