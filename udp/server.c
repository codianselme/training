#include <stdio.h>
#include <stdlib.h>

// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

// File function and bzero
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>

/* Taille du buffer utilisé pour envoyer le fichier
 * en plusieurs blocs
 */
#define BUFFERT 512


int main (int argc, char**argv)
{
	//Descripteur
	int socket_desc, socket_fd;

	struct sockaddr_in sock_serv, clt;
    
	char buf[BUFFERT];
	off_t count=0, n; // long type
	char filename[256];
    unsigned int taille=sizeof(struct sockaddr_in);
	
    // Variable pour la date
	time_t temps;
	struct tm* st_temps;
    
	// printf('Usage : ./executable <Port_Serveur>');

    if (argc != 2)
    {
		printf("Erreur!!! usage : %s <Port_Serveur>\n", argv[0]);
		return EXIT_FAILURE;
	}

	socket_fd = socket(AF_INET,SOCK_DGRAM,0);
	if (socket_fd == -1)
	{
        perror("Echec de création du socket ");
        return EXIT_FAILURE;
	}
    
	//preparation de l'adresse de la socket destination
	taille=sizeof(struct sockaddr_in);
	bzero(&sock_serv, taille);
	
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(atoi(argv[1]));
	sock_serv.sin_addr.s_addr=inet_addr("127.0.0.1");
	//sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);

	//Affecter une identité au socket
	if(bind(socket_fd,(struct sockaddr*)&sock_serv,taille)==-1)
	{
		perror("Echec de liaison ");
		return EXIT_FAILURE;
	}

	temps = time(NULL);
	st_temps = localtime(&temps); 
	bzero(filename, 256);
	sprintf(filename,"fichier_copié.%d.%d.%d.%d.%d.%d",st_temps->tm_mday,st_temps->tm_mon+1,1900+st_temps->tm_year,st_temps->tm_hour,st_temps->tm_min,st_temps->tm_sec);
	printf("Création du fichier de sortie : %s\n",filename);
    
	//ouverture du fichier
	if((socket_desc=open(filename,O_CREAT|O_WRONLY|O_TRUNC,0600))==-1)
	{
		perror("Echec d'ouverture");
		return EXIT_FAILURE;
	}
    
	//preparation de l'envoie
	bzero(&buf,BUFFERT);
    n=recvfrom(socket_fd, &buf, BUFFERT, 0, (struct sockaddr *)&clt, &taille);
	while(n)
	{
		printf("%lld de données reçues \n", n);
		if(n==-1)
		{
			perror("Echec de lecture");
			return EXIT_FAILURE;
		}
		count+=n;
		write(socket_desc,buf,n);
		bzero(buf,BUFFERT);
        n=recvfrom(socket_fd,&buf,BUFFERT,0,(struct sockaddr *)&clt,&taille);
	}
    
	printf("%lld octets transférés \n", count);
    
    close(socket_fd);
    close(socket_desc);
	return EXIT_SUCCESS;

}
