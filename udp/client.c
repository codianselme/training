#include <stdio.h>
#include <stdlib.h>

// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

// File function and bzero
#include <fcntl.h>
#include <unistd.h>
#include <strings.h> 

/* Taille du buffer utilise pour envoyer le fichier en plusieurs blocs */
#define BUFFERT 512



int main (int argc, char**argv)
{
	struct timeval start, stop, delta;
    int socket_desc, socket_fd, l;
    char buf[BUFFERT];
    off_t count=0, m, sz; //long
	long int n;
	struct stat buffer;
	struct sockaddr_in sock_serv;
    
	if (argc != 4)
	{
		printf("Erreur!!! usage : %s <Ip_Serveur> <Port_Serveur> <Nom_du_fichier>\n",argv[0]);
		return EXIT_FAILURE;
	}

	socket_desc = socket(AF_INET,SOCK_DGRAM,0);
	if (socket_desc == -1)
	{
        perror("Echec de création du socket");
        return EXIT_FAILURE;
	}
    
    //preparation de l'adresse de la socket destination
	l=sizeof(struct sockaddr_in);
	bzero(&sock_serv, l);
	
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &sock_serv.sin_addr)==0)
	{
		printf("Adresse IP non valide \n");
		return EXIT_FAILURE;
	}
        
	if ((socket_fd = open(argv[3], O_RDONLY))==-1)
	{
		perror("Echec d'ouverture");
		return EXIT_FAILURE;
	}
    
	//taille du fichier
	if (stat(argv[3], &buffer)==-1)
	{
		perror("Echec de stat ");
		return EXIT_FAILURE;
	}
	else
		sz=buffer.st_size;
    
	//preparation de l'envoie
	bzero(&buf,BUFFERT);
    
	gettimeofday(&start,NULL);
    n=read(socket_fd,buf,BUFFERT);
	while(n)
	{
		if(n==-1)
		{
			perror("Echec de lecture");
			return EXIT_FAILURE;
		}
		m=sendto(socket_desc,buf,n,0,(struct sockaddr*)&sock_serv,l);
		if(m==-1)
		{
			perror("Echec d'envoi");
			return EXIT_FAILURE;
		}
		count+=m;
		bzero(buf,BUFFERT);
        n=read(socket_fd,buf,BUFFERT);
	}
	
	//Pour debloquer le serveur
	m=sendto(socket_desc,buf, 0, 0, (struct sockaddr*)&sock_serv, l);
	gettimeofday(&stop,NULL);
    
	printf("Nombre d'octets transférés : %lld\n", count);
	printf("Sur une taille total de : %lld \n", sz);
	printf("Pour une durée total de : %ld.%d \n", delta.tv_sec, delta.tv_usec);
    
    close(socket_desc);
    close(socket_fd);
	return EXIT_SUCCESS;
}
