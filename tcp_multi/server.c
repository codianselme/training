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


int tcp_listen(int host, int port, int backlog);
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

int main(int argc, char *argv[])
{
	struct sockaddr_in  client;
	int sock1, sock2, sock3, sock4;
	int input, child;
	//int n, result;
	socklen_t len;
	//char buf[100];

	printf("Entrer le Numéro de Port : ");
	scanf("%d", &input);

	sock1 = tcp_listen(INADDR_ANY, input,5);
	sock2 = tcp_listen(INADDR_ANY, input+1,6);
	len = sizeof(client);
	if((pid=fork())!=0){
		exit(0);
	}
	else{
		while(1){
			sock3 = accept(sock1, (struct sockaddr*)&client, &len);
			sock4 = accept(sock2, (struct sockaddr*)&client, &len);
			
			if(sock3 < 0){
				continue;
			}
			num_user++;
			pid1 = fork();
			
			if(pid1 < 0){
				printf("\n");
				return -1;
			}
			else if(pid1 > 0){
				send(sock3, "Protocol TCP", 100, 0);
				child = getpid();
				printf("\nN° du pid : %d\n",child);
				//
				send(sock3,&child,100,0);
				printf("Utilisateur %d\n\n",num_user);
				main_select(sock3,sock4);
			}
		}
	}
	return 0; 
}


int tcp_listen(int host, int port, int backlog){
	int sd = 0;
	sd = socket(AF_INET, SOCK_STREAM,0);
	if(sd == -1){
		perror("socket échec !!");
		exit(1);
	}
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if(bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		perror("Echec de liaison !!");
		exit(1);
	}
	listen(sd,backlog);
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

