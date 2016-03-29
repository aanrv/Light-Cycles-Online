#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// memset
#include <sys/socket.h>		// socket, bind
#include <netinet/in.h>		// sockaddr_in
#include <arpa/inet.h>		// inet_ntop
#include "h.h"			// PLAYER_1, PLAYER_2

enum {APPNAME, PORTNUM};

const long PORTMIN = 1024;		// largest port num allowed
const long PORTMAX = 65535;		// smallest port num allowed
const short NUMPLAYERS = 2;		// maximum number of players allowed
const unsigned short DEFPORT = 1337;	// default port to use if not provided

/**
 * Creates server and assigns socket and address.
 */
void createserver(int* servsock, struct sockaddr_in* servaddr, unsigned short port);

/**
 * Waits for `numplayers` players to connect (currently only 2, but param is still there for future expansion).
 */
void waitforplayers(int servsock, int* p1sock, int* p2sock, struct sockaddr_in* p1addr, struct sockaddr_in* p2addr, int numplayers);

/**
 * Converts an str to an unsigned short with port-specific error checking.
 */
unsigned short strtoport(char* port);

/**
 * Exits with failure status and displays message.
 * If errno is set, perror is used,
 * else message is printed directly to stderr.
 */
void exitwerror(const char* msg, int errno);

int main(int argc, char** argv) {
	const unsigned short DEFPORT = 1337;		// default port

	// create server
	int servsock;
	struct sockaddr_in servaddr;
	unsigned short port = DEFPORT;
	createserver(&servsock, &servaddr, port);

	puts("Server started.\nWaiting for players to connect...");

	// wait for players to connect, assign appropriate values
	int p1sock;
	int p2sock;
	struct sockaddr_in p1addr;
	struct sockaddr_in p2addr;
	waitforplayers(servsock, &p1sock, &p2sock, &p1addr, &p2addr, NUMPLAYERS);

	return 0;
}

void createserver(int* servsock, struct sockaddr_in* servaddr, unsigned short port) {
	if ((*servsock = socket(AF_INET, SOCK_STREAM, 0)) == -1) exitwerror("socket", 1);

	int enable = 1;
	if (setsockopt(*servsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) == -1)
		fprintf(stderr, "setsockopt: Failed, socket will no reuse port.\n");

	memset(servaddr, 0, sizeof (*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	if (port == 0) {				// if port is 0, error is assumed to have occured
		fprintf(stderr, "Port invalid: must be between %lu and %lu.\n", PORTMIN, PORTMAX);
		exit(EXIT_FAILURE);
	}
	servaddr->sin_port = htons(port);

	if (bind(*servsock, (struct sockaddr*) servaddr, sizeof (*servaddr)) == -1) exitwerror("bind", 1);
}

void waitforplayers(int servsock, int* p1sock, int* p2sock, struct sockaddr_in* p1addr, struct sockaddr_in* p2addr, int numplayers) {
	// set server to passive
	const int BACKLOG = 5;
	if (listen(servsock, BACKLOG) == -1) exitwerror("listen", 1);

	// wait for connection from 2 players

	memset(p1addr, 0, sizeof (*p1addr));
	memset(p2addr, 0, sizeof (*p2addr));

	socklen_t p1len = sizeof (*p1addr);
	socklen_t p2len = sizeof (*p2addr);

	char p1buffer[INET_ADDRSTRLEN];
	char p2buffer[INET_ADDRSTRLEN];

	*p1sock = accept(servsock, (struct sockaddr*) p1addr, &p1len);
	if (*p1sock == -1) exitwerror("accept (Player1)", 1);

	if (inet_ntop(AF_INET, &(p1addr->sin_addr), p1buffer, INET_ADDRSTRLEN) == NULL) fprintf(stderr, "P1: Couldn't convert.\n");
	else printf("Player 1 (%s:%u) connected.\n", p1buffer, ntohs(p1addr->sin_port));


	*p2sock = accept(servsock, (struct sockaddr*) p2addr, &p2len);
	if (*p2sock == -1) exitwerror("accept (Player2)", 1);

	if (inet_ntop(AF_INET, &(p2addr->sin_addr), p2buffer, INET_ADDRSTRLEN) == NULL) fprintf(stderr, "P2: Couldn't convert.\n");
	else printf("Player 2 (%s:%u) connected.\n", p2buffer, ntohs(p2addr->sin_port));


	// wait after both have connected to send notifications
	char p1 = PLAYER_1;
	if (send(*p1sock, &p1, 1, 0) == -1) exitwerror("send (player1)", 1);
	char p2 = PLAYER_2;
	if (send(*p2sock, &p2, 1, 0) == -1) exitwerror("send (player2)", 1);
}

unsigned short strtoport(char* str) {
	const int BASE = 10;		// conversion base
	long out = strtol(str, NULL, BASE);

	if (out < PORTMIN || out > PORTMAX) out = 0;
	return (unsigned short) out;
}

void exitwerror(const char* msg, int useserrno) {
	if (useserrno) perror(msg);
	else fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

