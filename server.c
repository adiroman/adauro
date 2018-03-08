#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define SERVER_PORT		5681
#define SERVER_ADDRESS	"127.0.0.1"
#define SERVER_BACKLOG	15		// numarul maxim de clienti din coada de conexiuni
#define MAX_BUFFER		1024
#define MAX_USERNAME	50
#define MAX_PASSWORD	100
#define MAX_SOCK		1		// numar maxim de socket-uri (numarul maxim de utilizatori care se pot conecta la server)	

int sock_clienti[MAX_SOCK];		// descriptorii de fisier pentru toate socket-urile deschise
int nrClienti;					// numarul de clienti conectati (nr de socket-uri deschise)
int grup[MAX_SOCK];				// fiecare intrare va memora numarul grupului din care face parte utilizatorul respectiv

int server_sock;

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
		printf("Eroare la citirea campului tip_mesaj din header [sock = %d]\n", fd);
		exit(EXIT_FAILURE);
	}

	return buffer[0];
}

int citireHeader_Lungime(int fd){
	char buffer[4];
	int nr;

	if(read(fd, buffer, 4) < 0){
		printf("Eroare la citirea campului len din header [sock = %d]\n", fd);
		exit(EXIT_FAILURE);
	}

	nr = atoi(buffer);

	return nr;
}

void trimitereHeader(head m_head, int client_sock){
	char buffer[4];

	sprintf(buffer, "%c", m_head.tip_mesaj);
	if(write(client_sock, buffer, 1) < 0){
		printf("Eroare la transmiterea campului tip_mesaj din header [sock = %d]\n", client_sock);
		exit(EXIT_FAILURE);
	}

	sprintf(buffer, "%d", m_head.len);
	if(write(client_sock, buffer, 4) < 0){
		printf("Eroare la transmiterea campului len din header [sock = %d]\n", client_sock);
		exit(EXIT_FAILURE);
	}
}

char* citireBody(int fd, int len){

	char* buffer;
	buffer = malloc (sizeof(char) * (len + 1));

	if(read(fd, buffer, len) < 0){
		printf("Eroare la citirea campului mesaj din body [sock = %d]\n", fd);
		exit(EXIT_FAILURE);
	}

	buffer[len] = '\0';
	return buffer;
}

void trimitereBody(body m_body, int client_sock){

	if(write(client_sock, m_body.mesaj, strlen(m_body.mesaj)) < 0){
		printf("Eroare la transmiterea campului mesaj din body [sock = %d]\n", client_sock);
		exit(EXIT_FAILURE);
	}
}

bool utilizatorValid(char *nume_utilizator){

	return true;
}

bool parolaValida(char *nume_utilizator, char *parola){

	return true;
}

int grupConversatie(char *nume_utilizator){

	// Functia va cauta numarul grupului de conversatie asociat utilizatorului dat prin nume_utilizator
	// Functia va returna un numar natural pozitiv (0, 1, ...) care va reprezenta grupul

	return 1;
}

void stergereClient(int socket){
	int i, k;

	for (i = 0; i < nrClienti; ++i)
		if (sock_clienti[i] == socket){

			for (k =  i; k < nrClienti - 1; ++k){
				sock_clienti[k] = sock_clienti[k + 1];
				grup[k] = grup[k + 1];
			}

			--nrClienti;
			break;
		}
}

int getPos (int socket){
	int i;

	// functia returneaza pozitia (indicele) din tabloul sock_clienti la care se afla clientul cu socketul primit ca parametru

	for (i = 0; i < nrClienti; ++i){
		if (sock_clienti[i] == socket) return i;
	}

	return -1; // nu trebuie sa ajung aici
}




void *procesare_cerere(void *arg){

	int client_sock = *((int*)arg);
	head client_head, server_head;
	body client_body;

	char nume_utilizator[MAX_USERNAME];
	int i, grup_utilizator;

	// astept ca clientul sa imi trimita un nume de utilizator valid (mesaj de tip 'u')
	while (true){
		// citesc header-ul
		client_head.tip_mesaj = citireHeader_TipMesaj(client_sock);
		client_head.len = citireHeader_Lungime(client_sock);

		// citesc numele utilizatorului din corpul pachetului
		client_body.mesaj = citireBody(client_sock, client_head.len);

		if(utilizatorValid(client_body.mesaj)){
			// memorez numele de utilizator si grupul pentru a putea fi folosit in continuare in sesiune
			strcpy(nume_utilizator, client_body.mesaj);
			grup_utilizator = grupConversatie(nume_utilizator);
			// grup_utilizator = client_head.len;		PENTRU TESTARE

			//trimit mesaj de confirmare catre client
			server_head.tip_mesaj = 'c';
			server_head.len = 0;
			trimitereHeader(server_head, client_sock);
			break;
		}

		// daca numele de utilizator nu e valid, trimit un mesaj de eroare catre client si recitesc pachetul
		server_head.tip_mesaj = 'e';
		server_head.len = 11;
		trimitereHeader(server_head, client_sock);
	}

	// astept ca clientul sa imi trimita o parola valida (mesajde tip 'p')
	while(true){
		client_head.tip_mesaj = citireHeader_TipMesaj(client_sock);
		client_head.len = citireHeader_Lungime(client_sock);
		client_body.mesaj = citireBody(client_sock, client_head.len);

		if(parolaValida(nume_utilizator, client_body.mesaj)){
			server_head.tip_mesaj = 'c';
			server_head.len = 0;
			trimitereHeader(server_head, client_sock);

			// memorez grupul din care face parte utilizatorul
			grup[getPos(client_sock)] = grup_utilizator;

			break;
		}

		// parola invalida
		server_head.tip_mesaj = 'e';
		server_head.len = 12;
		trimitereHeader(server_head, client_sock);
	}

	printf("Client <%s> conectat [sock = %d]\n", nume_utilizator, client_sock);



	while(true){

		// dupa ce m-am autentificat, pot sa primesc mesaje de tip text sau cerere de deconectare

		client_head.tip_mesaj = citireHeader_TipMesaj(client_sock);
		client_head.len = citireHeader_Lungime(client_sock);

		if (client_head.tip_mesaj == 'm'){			// mesaj de tip text
			client_body.mesaj = citireBody(client_sock, client_head.len);

			// trimit confirmare de primire catre client
			server_head.tip_mesaj = 'c';
			server_head.len = 0;
			trimitereHeader(server_head, client_sock);

			// trimit mesajul catre toti utilizatorii conectati
			server_head.tip_mesaj = 'm';
			server_head.len = client_head.len;

			for (i = 0; i < nrClienti; ++i){
				if(grup_utilizator == grup[i]){
					trimitereHeader(server_head, sock_clienti[i]);
					trimitereBody(client_body, sock_clienti[i]);
				}
			}
		}
		else if (client_head.tip_mesaj == 'd'){		// cerere de deconectare

			// trimit confirmare clientului
			server_head.tip_mesaj = 'c';
			server_head.len = 0;
			trimitereHeader(server_head, client_sock);

			// astept confirmarea de la client
			client_head.tip_mesaj = citireHeader_TipMesaj(client_sock);
			client_head.len = citireHeader_Lungime(client_sock);


			// dupa ce am primit confirmarea de la client, deconectez clientul de la socket si inchei firul de executie
			if (client_head.tip_mesaj == 'c'){
				stergereClient(client_sock);
				break;
			}
		}
		// daca protocolul e respectat, dupa autentificare pot sa am doar mesaje de tipul 'd' si 'm'
	}

	printf("Client <%s> deconectat\n", nume_utilizator);

	pthread_exit(NULL);
}

int main(int argc, char const *argv[]){

	int client_sock;
	struct sockaddr_in my_addr;

	// creez socket-ul
	if((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Eroare la crearea socket-ului\n");
		exit(EXIT_FAILURE);
	}

	// asociez socket-ului creat o adresa IP si un port
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVER_PORT);
	inet_pton(PF_INET, SERVER_ADDRESS, &my_addr.sin_addr);

	if(bind(server_sock, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0){
		printf("Eroare la executarea apelului bind\n");
		exit(EXIT_FAILURE);
	}

	// anunt sistemul de operare ca doresc sa accept conexiuni
	if(listen(server_sock, SERVER_BACKLOG) < 0){
		printf("Eroare la executarea apelului listen\n");
		exit(EXIT_FAILURE);
	}

	socklen_t rlen = sizeof(my_addr);

	printf("Serverul e pregatit de lucru...\n\n");

	while(true){

		if(nrClienti < MAX_SOCK){

			// preiau o cerere de la un proces client din coada de conexiuni
			if((client_sock = accept(server_sock, (struct sockaddr*)&my_addr, &rlen)) < 0){
				printf("Eroare la executarea apelului accept\n");
				exit(EXIT_FAILURE);
			}

			// pun socket-ul in tabloul de socket-uri
			sock_clienti[nrClienti] = client_sock;
			++nrClienti;

			pthread_t thread_id;

			if(pthread_create(&thread_id, NULL, &procesare_cerere, &client_sock) != 0){
				printf("Eroare la crearea unui fir de executie nou\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	// pthread_join !!!!!!!

	return -1;
}
