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

#define SERVER_PORT		5678
#define SERVER_ADDRESS	"127.0.0.1"
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

	while(true);

	return 0;
}
