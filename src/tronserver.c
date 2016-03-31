#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// memset
#include <sys/socket.h>		// socket, bind
#include <netinet/in.h>		// sockaddr_in
#include <arpa/inet.h>		// inet_ntop
#include "player.h"		// Direction
#include "h.h"			// PLAYER_1, PLAYER_2

enum {APPNAME, PORTNUM};

const long PORTMIN 		= 1024;		// smallest port num allowed
const long PORTMAX 		= 65535;	// largest port num allowed
const unsigned short DEFPORT	= 1337;		// default port to use if not provided

/* Creates server and assigns socket and address. */
void createserver(int* servsock, struct sockaddr_in* servaddr, unsigned short port);

/* Waits for `numplayers` players to connect (currently only 2, but param is still there for future expansion). */
void waitforplayers(int servsock, int* sockarr, struct sockaddr_in* addrarr);

/* Send players their corresponding player numbers. */
void sendplayernums(int* socks);

/* Recieves variables from each connected player. */
void recvvars(char* dirs, int* clisocks);

/* Sends variables (direction, etc.) to each connected player. */
void sendvars(int* socks, char* buffer);

/* Converts an str to an unsigned short with port-specific error checking. */
unsigned short strtoport(char* port);

/**
 * Exits with failure status and displays message.
 * If errno is set, perror is used,
 * else message is printed directly to stderr.
 */
void exitwerror(const char* msg, int useserrno);

int main(int argc, char** argv) {
	const unsigned short DEFPORT = 1337;		// default port

	puts("Starting server.");
	// create server
	int servsock;
	struct sockaddr_in servaddr;
	unsigned short port = DEFPORT;
	createserver(&servsock, &servaddr, port);

	puts("Server started.\nWaiting for players to connect...");

	// wait for players to connect, assign appropriate values
	int sockarr[NUMPLAYERS];
	struct sockaddr_in addrarr[NUMPLAYERS];
	waitforplayers(servsock, sockarr, addrarr);	// wait for players to connect
	
	puts("Sending players numbers.");
	// notify players of their player numbers
	sendplayernums(sockarr);			// send notification to each player
	puts("Done.");
	
	// inital direction values
	char dirbuffer[SCPSIZE];
	dirbuffer[PLAYER_1] = LEFT;
	dirbuffer[PLAYER_2] = RIGHT;

	for (;;) {
		sendvars(sockarr, dirbuffer);		// send starting directions
		recvvars(dirbuffer, sockarr);		// recieve client information coming back to server socket
	}

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

void waitforplayers(int servsock, int* sockarr, struct sockaddr_in* addrarr) {
	// set server to passive
	const int BACKLOG = 5;
	if (listen(servsock, BACKLOG) == -1) exitwerror("listen", 1);

	// wait for connection from 2 players
	memset(addrarr, 0, sizeof (struct sockaddr_in) * NUMPLAYERS);
	socklen_t p1len = sizeof (struct sockaddr_in);
	socklen_t p2len = sizeof (struct sockaddr_in);

	char p1buffer[INET_ADDRSTRLEN];
	char p2buffer[INET_ADDRSTRLEN];

	sockarr[PLAYER_1] = accept(servsock, (struct sockaddr*) &addrarr[PLAYER_1], &p1len);
	if (sockarr[PLAYER_1] == -1) exitwerror("accept (Player1)", 1);

	if (inet_ntop(AF_INET, &(addrarr[PLAYER_1].sin_addr), p1buffer, INET_ADDRSTRLEN) == NULL) fprintf(stderr, "P1: Couldn't convert.\n");
	else printf("Player 1 (%s:%u) connected.\n", p1buffer, ntohs(addrarr[PLAYER_1].sin_port));

	sockarr[PLAYER_2] = accept(servsock, (struct sockaddr*) &addrarr[PLAYER_2], &p2len);
	if (sockarr[PLAYER_2] == -1) exitwerror("accept (Player2)", 1);

	if (inet_ntop(AF_INET,&(addrarr[PLAYER_2].sin_addr), p2buffer, INET_ADDRSTRLEN) == NULL) fprintf(stderr, "P2: Couldn't convert.\n");
	else printf("Player 2 (%s:%u) connected.\n", p2buffer, ntohs(addrarr[PLAYER_2].sin_port));
}

void sendplayernums(int* socks) {
	// wait after both have connected to send notifications
	char p1 = PLAYER_1;
	char p2 = PLAYER_2;
	if (send(socks[PLAYER_1], &p1, 1, 0) == -1) exitwerror("send (player1)", 1);
	if (send(socks[PLAYER_2], &p2, 1, 0) == -1) exitwerror("send (player2)", 1);
}

void recvvars(char* dirs, int* clisocks) {
	char recvbuffer[CSPSIZE];
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) {		// recieve from all players
		int currsock = clisocks[i];
		if (recv(currsock, recvbuffer, CSPSIZE, 0) == -1) exitwerror("recvvars", 1);
		
		if (i == PLAYER_1) dirs[PLAYER_1] = recvbuffer[PDIR];	// set direction
		else if (i == PLAYER_2) dirs[PLAYER_2] = recvbuffer[PDIR];
		else exitwerror("wottttt", 0);
	}
}

void sendvars(int* socks, char* buffer) {
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) {
		if (send(socks[i], buffer, SCPSIZE, 0) == -1) exitwerror("sendvars", 1);
	}
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

