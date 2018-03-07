#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define SERVER_PORT		5678
#define SERVER_ADDRESS	"127.0.0.1"
#define SERVER_BACKLOG	4

typedef struct head{
	char tip_mesaj;
	int len;
} head;

typedef struct body{
	char *mesaj;
} body;

int main(int argc, char const *argv[]){

	int server_sock, client_sock;
	struct sockaddr_in my_addr;

	// creez socket-ul
	if((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("Eroare la crearea socket-ului\n");
		exit(EXIT_FAILURE);
	}

	// asociez socket-ului creat o adresa IP si un port
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVER_PORT);
	inet_pton(PF_INET, SERVER_ADDRESS, &my_addr.sin_addr);

	if(bind(server_sock, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0){
		perror("Eroare la executarea apelului bind\n");
		exit(EXIT_FAILURE);
	}

	// anunt sistemul de operare ca doresc sa accept conexiuni
	if(listen(server_sock, SERVER_BACKLOG) < 0){
		perror("Eroare la executarea apelului listen\n");
		exit(EXIT_FAILURE);
	}

	socklen_t rlen = sizeof(my_addr);

	while(true){

		// preiau o cerere de la un proces client din coada de conexiuni
		if((client_sock = accept(server_sock, (struct sockaddr*)&my_addr, &rlen)) < 0){
			perror("Eroare la executarea apelului accept\n");
			exit(EXIT_FAILURE);
		}

		

		printf("Client conectat %d\n", client_sock);
	}

	return -1;
}