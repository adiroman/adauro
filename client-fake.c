/* NU MODIFICA !!!!!!!!!!!!!!!!!!!!
*/

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

void trimitereBody(body m_body, int client_sock){

	if(write(client_sock, m_body.mesaj, strlen(m_body.mesaj)) < 0){
		perror("Eroare la transmiterea campului mesaj din body\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char const *argv[])
{
	int server_sock, client_sock;
	if((server_sock=socket(PF_INET,SOCK_STREAM,0))<0){
		printf("Eroare 1\n");
	}

	struct sockaddr_in local_addr;
	struct sockaddr_in server_addr;

	set_addr(&local_addr, NULL, INADDR_ANY, 0);
	bind(server_sock, (struct sockaddr*)&local_addr, sizeof(local_addr));

	set_addr(&server_addr, SERVER_ADDRESS, 0, SERVER_PORT);

	connect(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

	head my_head;
	body my_body;

	

	char buf[1000];
	char mesaj[1000];
	char *tmpBuffer;
	int lungimeMesaj;
	

	my_head.tip_mesaj = 'u';
	my_head.len = 18;
	sprintf(buf, "%c", my_head.tip_mesaj);
	write(server_sock, buf, 1);
	sprintf(buf, "%d", my_head.len);
	write(server_sock, buf, 4);
	sprintf(buf, "utilizator_fals");
	write(server_sock, buf, strlen(buf));


	printf("Tip mesaj: %c\n", citireHeader_TipMesaj(server_sock));
	printf("Lungime mesaj: %d\n", citireHeader_Lungime(server_sock));

	my_head.tip_mesaj = 'p';
	my_head.len = 10;
	sprintf(buf, "%c", my_head.tip_mesaj);
	write(server_sock, buf, 1);
	sprintf(buf, "%d", my_head.len);
	write(server_sock, buf, 4);
	sprintf(buf, "1234567890");
	write(server_sock, buf, 10);


	printf("Tip mesaj: %c\n", citireHeader_TipMesaj(server_sock));
	printf("Lungime mesaj: %d\n", citireHeader_Lungime(server_sock));

	my_head.tip_mesaj = 'm';
	sprintf(mesaj, "Mesaj fals de la utilizator fals");
	my_head.len = strlen(mesaj);
	sprintf(buf, "%c", my_head.tip_mesaj);
	write(server_sock, buf, 1);
	sprintf(buf, "%d", my_head.len);
	write(server_sock, buf, 4);
	write(server_sock, mesaj, my_head.len);
	

	printf("Tip mesaj: %c\n", citireHeader_TipMesaj(server_sock));
	printf("Lungime mesaj: %d\n", citireHeader_Lungime(server_sock));


	printf("Tip mesaj: %c\n", citireHeader_TipMesaj(server_sock));
	lungimeMesaj = citireHeader_Lungime(server_sock);
	printf("Lungime mesaj: %d\n", lungimeMesaj);


	tmpBuffer = citireBody(server_sock, lungimeMesaj);
	printf("%s\n", tmpBuffer);


	//%%%%%%%%%%%%%%%5

	my_head.tip_mesaj = 'm';
	sprintf(mesaj, "Mesaj fals de la utilizator fals");
	my_head.len = strlen(mesaj);
	sprintf(buf, "%c", my_head.tip_mesaj);
	write(server_sock, buf, 1);
	sprintf(buf, "%d", my_head.len);
	write(server_sock, buf, 4);
	write(server_sock, mesaj, my_head.len);
	

	printf("Tip mesaj: %c\n", citireHeader_TipMesaj(server_sock));
	printf("Lungime mesaj: %d\n", citireHeader_Lungime(server_sock));


	printf("Tip mesaj: %c\n", citireHeader_TipMesaj(server_sock));
	lungimeMesaj = citireHeader_Lungime(server_sock);
	printf("Lungime mesaj: %d\n", lungimeMesaj);


	tmpBuffer = citireBody(server_sock, lungimeMesaj);
	printf("%s\n", tmpBuffer);

	return 0;
}
