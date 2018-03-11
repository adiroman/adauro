#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
int fd[2];

#define SERVER_PORT		5679
#define SERVER_ADDRESS	"192.168.0.110"
#define SERVER_MAX_CONNECTION	4

int set_addr ( struct sockaddr_in *addr , char *name, u_int32_t inaddr, short sin_port) {
	struct hostent *h ;
	memset ( ( void *) addr , 0 , sizeof (* addr ) ) ;
	addr->sin_family = PF_INET ;
	if ( name != NULL) {
		h = gethostbyname ( name ) ;
		if ( h == NULL)
			return -1;
		addr->sin_addr.s_addr=*(u_int32_t *) h->h_addr_list [ 0 ] ;
	} else
		addr->sin_addr.s_addr = htonl (inaddr);
	
	addr->sin_port = htons(sin_port) ;
	return 0;
}

typedef struct head{
	char tip_mesaj;
	int len;
} head;

typedef struct body{
	char *mesaj;
} body;

char citireHeader_TipMesaj(int fd){
	char buffer[1];

	if(read(fd, buffer, 1) < 0){
		perror("Eroare la citirea campului tip_mesaj din header\n");
		exit(EXIT_FAILURE);
	}

	return buffer[0];
}
int citireHeader_Lungime(int fd){
	char buffer[4];
	int nr;

	if(read(fd, buffer, 4) < 0){
		perror("Eroare la citirea campului len din header\n");
		exit(EXIT_FAILURE);
	}

	nr = atoi(buffer);
	
	return nr;
}
char* citireBody(int fd, int len){

	char* buffer;
	buffer = malloc (sizeof(char) * (len + 1));

	if(read(fd, buffer, len) < 0){
		perror("Eroare la citirea campului mesaj din body\n");
		exit(EXIT_FAILURE);
	}

	buffer[len] = '\0';
	return buffer;
}
void trimitereHeader(head m_head, int client_sock){
	char buffer[4];

	sprintf(buffer, "%c", m_head.tip_mesaj);
	if(write(client_sock, buffer, 1) < 0){
		perror("Eroare la transmiterea campului tip_mesaj din header\n");
		exit(EXIT_FAILURE);
	}

	sprintf(buffer, "%d", m_head.len);
	if(write(client_sock, buffer, 4) < 0){
		perror("Eroare la transmiterea campului len din header\n");
		exit(EXIT_FAILURE);
	}
}
void trimitereBody(body m_body, int client_sock){

	if(write(client_sock, m_body.mesaj, strlen(m_body.mesaj)) < 0){
		perror("Eroare la transmiterea campului mesaj din body\n");
		exit(EXIT_FAILURE);
	}
}

int sendCredentials(int server_sock,char type){
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(server_sock,&rfds);
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	head my_head;
	
	char buf[1000];
	char mesaj[1000];
	while(true){
		if(type == 'u')
			printf("Introdu numele de utilizator:");
		else 
			if(type == 'p')
				printf("Introdu parola:");
			else 
				return -1;
		fgets(buf,1000,stdin);
		buf[strlen(buf)-1]=buf[strlen(buf)];
		sprintf(mesaj,"%s",buf);
		my_head.tip_mesaj = type;
		my_head.len = strlen(buf);
		sprintf(buf, "%c", my_head.tip_mesaj);
		write(server_sock, buf, 1);
		sprintf(buf, "%d", my_head.len);
		write(server_sock, buf, 4);
		write(server_sock, mesaj, strlen(mesaj));
		
		int retval = select(server_sock+1,&rfds,NULL,NULL,&tv);
		if(retval == -1){
			perror("Eroare select:\n");
		}else 
			if(retval){
				if(citireHeader_TipMesaj(server_sock) != 'c'){
					citireHeader_Lungime(server_sock);
					if(type == 'u')
						printf("Nume utilizator gresit\n");
					else printf("Parola gresita\n");
					continue;			
				}
				else{ 
					citireHeader_Lungime(server_sock);
					return 1;
				}
			}else{
				printf("No data within 10 seconds\n");
				return 0;
			}
	}
}

head listener_tip_mesaj(int server_sock){
	head my_head;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(server_sock,&rfds);
	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	int retval = select(server_sock+1,&rfds,NULL,NULL,&tv);
	
	my_head.tip_mesaj = NULL;
	my_head.len = NULL;
	if(retval){
		my_head.tip_mesaj = citireHeader_TipMesaj(server_sock);
		my_head.len = citireHeader_Lungime(server_sock);
		return my_head;
	}
	return my_head;
}

void primire_mesaj(int server_sock,int pipefd){
	head my_head;
	while(true){
		my_head = listener_tip_mesaj(server_sock);
		if(my_head.tip_mesaj=='m'){
			char * c = citireBody(server_sock,my_head.len);
			printf("%s\n",c);
		}
		if(my_head.tip_mesaj=='c'){
			trimitereHeader(my_head,pipefd);
		}
	}
}

int main(int argc, char const *argv[]){
	int server_sock;
	if((server_sock=socket(PF_INET,SOCK_STREAM,0))<0){
		perror("Eroare creare socket:\n");
	}

	struct sockaddr_in local_addr;
	struct sockaddr_in server_addr;

	set_addr(&local_addr, NULL, INADDR_ANY, 0);
	if(bind(server_sock, (struct sockaddr*)&local_addr, sizeof(local_addr))<0){
		perror("Eroare bind:\n");
		exit(1);
	}
	set_addr(&server_addr, SERVER_ADDRESS, 0, SERVER_PORT);
	
	if(connect(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		perror("Conectare nereusita:\n");
		exit(1);
	}
	
	head my_head;
	char buf[1000];
	char mesaj[1000];

	if((sendCredentials(server_sock,'u') == 0) || (sendCredentials(server_sock,'p') == 0)){
		printf("Nu se poate efectua conexiunea la server\n");
		exit(1);
	}

	printf("\nPress ^[ to leave chat room\n\n");

	int pid;
	if(pipe(fd)==-1){
		perror("nu pot creea pipe");
	}
	if((pid=fork())==-1){
		perror("nu pot creea proces");
	}
	if(pid==0){
		close(fd[0]);
		primire_mesaj(server_sock,fd[1]);
		exit(0);
	}
	close(fd[1]);
	while(true){
			my_head.tip_mesaj = 'm';
			fgets(mesaj,1000,stdin);
			if(mesaj[0] != 27){
				mesaj[strlen(mesaj)-1]=mesaj[strlen(mesaj)];
				my_head.len = strlen(mesaj);
				sprintf(buf, "%c", my_head.tip_mesaj);
				write(server_sock, buf, 1);
				sprintf(buf, "%d", my_head.len);
				write(server_sock, buf, 4);
				write(server_sock, mesaj, my_head.len);
				my_head = listener_tip_mesaj(fd[0]);
				if(my_head.tip_mesaj=='c'){
					printf("confirmed\n");
				}
				else{
					printf("neconfirmat %c\n",my_head.tip_mesaj);
				}
			}else{
				my_head.tip_mesaj = 'd';
				my_head.len = 0;
				sprintf(buf, "%c", my_head.tip_mesaj);
				write(server_sock, buf, 1);
				sprintf(buf, "%d", my_head.len);
				write(server_sock, buf, 4);
				printf("aaaaaaaaaaa\n");
				my_head = listener_tip_mesaj(fd[0]);
				//printf("@@%c@@\n",my_head.tip_mesaj==0 ? 'n' : my_head.tip_mesaj);
				if(my_head.tip_mesaj == 'c'){
					printf("Are you sure you want to exit chat room?[y/n]:");
					char c = getchar();
					getchar();
					if(c == 'y'){
						printf("y\n");
						my_head.tip_mesaj = 'c';
						my_head.len = 0;
						sprintf(buf, "%c", my_head.tip_mesaj);
						write(server_sock, buf, 1);
						sprintf(buf, "%d", my_head.len);
						write(server_sock, buf, 4);
						my_head = listener_tip_mesaj(fd[0]);
						if(my_head.tip_mesaj == 'c'){
							printf("closed !%c!\n",my_head.tip_mesaj);
							exit(1);
						}
						printf("not closed !%c!\n",my_head.tip_mesaj);
					}else 
						if (c == 'n'){
						printf("n\n");
						my_head.tip_mesaj = 'm';
						my_head.len = 1;
						sprintf(buf, "%c", my_head.tip_mesaj);
						write(server_sock, buf, 1);
						sprintf(buf, "%d", my_head.len);
						write(server_sock, buf, 4);
						my_head = listener_tip_mesaj(fd[0]);
						if(my_head.tip_mesaj == 'c'){
							printf("confirmed !%c!\n",my_head.tip_mesaj);
							continue;
						}
						printf("unconfirmed !%c!\n",my_head.tip_mesaj);
					}			
				}
			}
		}
	return 0;
}