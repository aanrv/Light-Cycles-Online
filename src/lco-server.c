#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// memset
#include <sys/socket.h>		// socket, bind
#include <netinet/in.h>		// sockaddr_in
#include <arpa/inet.h>		// inet_ntop
#include <unistd.h>		// close
#include "player.h"		// Direction
#include "h.h"			// PLAYER_1, PLAYER_2

enum {APPNAME, PORTNUM};
enum CollisionType {NONE = -1, COLP1, COLP2, BOTH};

/* Creates server and assigns socket and address. */
void createserver(int* servsock, struct sockaddr_in* servaddr, unsigned short port);

/* Waits for `numplayers` players to connect (currently only 2, but param is still there for future expansion). */
void waitforplayers(int servsock, int* sockarr, struct sockaddr_in* addrarr);

/* Send players numbers after clients have connected. */
void sendplayernums(int* socks);

/* Recieve signal from clients. Return collision status. */
enum CollisionType recvclisig(char* dirs, int* clisocks);

/* Recieves variables from player. 
   Handles CS_STD signal. */
void recvvars(char* playerdir, int clisock);

/* Sends variables (direction, etc.) to each connected player. */
void sendvars(int* socks, char* buffer);

/* Send end signal to clients and close server.
   Handles CS_COL signal. */
void endclients(int* clisocks, char winner);

int main(int argc, char** argv) {
	// create server
	int servsock;
	struct sockaddr_in servaddr;
	unsigned short port = argc >= 2 ? strtoport(argv[PORTNUM]) : DEFPORT;
	if (port == 0) {
		printf("Port must be between %lu and %lu.\n", PORTMIN, PORTMAX);
		exitwerror("Invalid port.", EXIT_STD);
	}
	createserver(&servsock, &servaddr, port);

	printf("Server running on port %u.\n", port);
	// server will begin accepting clients again after each game
	for (;;) {
		puts("\nWaiting for players to connect...");

		// wait for players to connect, then assign appropriate values
		int sockarr[NUMPLAYERS];
		struct sockaddr_in addrarr[NUMPLAYERS];
		waitforplayers(servsock, sockarr, addrarr);

		puts("Game started.");

		// notify players of their player numbers
		sendplayernums(sockarr);

		// inital direction values
		char dirbuffer[SC_STDSIZE];
		dirbuffer[PLAYER_1] = RIGHT;
		dirbuffer[PLAYER_2] = LEFT;

		// keep recieving and sending until a collision signal is recieved
		int collided = -1;
		while (collided == -1) {
			sendvars(sockarr, dirbuffer);			// send directions
			collided = recvclisig(dirbuffer, sockarr);	// recieve client information coming back to server socket
		}

		// determine winner
		char winner;
		switch (collided) {
			case PLAYER_1: winner = PLAYER_2; break;
			case PLAYER_2: winner = PLAYER_1; break;
			default: winner = 3;
		}

		// game over, notify clients
		endclients(sockarr, winner);
		puts("Game over.");
	}

	return EXIT_SUCCESS;
}

void createserver(int* servsock, struct sockaddr_in* servaddr, unsigned short port) {
	if ((*servsock = socket(AF_INET, SOCK_STREAM, 0)) == -1) exitwerror("socket", EXIT_ERRNO);

	// allow port reuse
	int enable = 1;
	if (setsockopt(*servsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) == -1)
		fprintf(stderr, "setsockopt: Failed, socket will no reuse port.\n");

	memset(servaddr, 0, sizeof (*servaddr));
	servaddr->sin_family = AF_INET;
	servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	if (port == 0) {
		fprintf(stderr, "Port invalid: must be between %lu and %lu.\n", PORTMIN, PORTMAX);
		exit(EXIT_FAILURE);
	}
	servaddr->sin_port = htons(port);

	if (bind(*servsock, (struct sockaddr*) servaddr, sizeof (*servaddr)) == -1) exitwerror("bind", EXIT_ERRNO);
}

void waitforplayers(int servsock, int* sockarr, struct sockaddr_in* addrarr) {
	// set server to passive
	const int BACKLOG = 5;
	if (listen(servsock, BACKLOG) == -1) exitwerror("listen", EXIT_ERRNO);

	memset(addrarr, 0, sizeof (struct sockaddr_in) * NUMPLAYERS);
	socklen_t p1len = sizeof (struct sockaddr_in);
	socklen_t p2len = sizeof (struct sockaddr_in);

	char p1buffer[INET_ADDRSTRLEN];
	char p2buffer[INET_ADDRSTRLEN];

	// wait for connections from 2 players

	sockarr[PLAYER_1] = accept(servsock, (struct sockaddr*) &addrarr[PLAYER_1], &p1len);
	if (sockarr[PLAYER_1] == -1) exitwerror("accept (Player1)", EXIT_ERRNO);

	if (inet_ntop(AF_INET, &(addrarr[PLAYER_1].sin_addr), p1buffer, INET_ADDRSTRLEN) == NULL) fprintf(stderr, "P1: Couldn't convert.\n");
	else printf("Player 1 (%s:%u) connected.\n", p1buffer, ntohs(addrarr[PLAYER_1].sin_port));

	sockarr[PLAYER_2] = accept(servsock, (struct sockaddr*) &addrarr[PLAYER_2], &p2len);
	if (sockarr[PLAYER_2] == -1) exitwerror("accept (Player2)", EXIT_ERRNO);

	if (inet_ntop(AF_INET,&(addrarr[PLAYER_2].sin_addr), p2buffer, INET_ADDRSTRLEN) == NULL) fprintf(stderr, "P2: Couldn't convert.\n");
	else printf("Player 2 (%s:%u) connected.\n", p2buffer, ntohs(addrarr[PLAYER_2].sin_port));
}

void sendplayernums(int* socks) {
	char p1 = PLAYER_1;
	char p2 = PLAYER_2;
	if (send(socks[PLAYER_1], &p1, 1, 0) == -1) exitwerror("send (player1)", EXIT_ERRNO);
	if (send(socks[PLAYER_2], &p2, 1, 0) == -1) exitwerror("send (player2)", EXIT_ERRNO);
}

enum CollisionType recvclisig(char* dirs, int* clisocks) {
	enum CollisionType numcoll = -1;
	char sigtype;
	int i;
	// iterate through clients
	for (i = 0; i < NUMPLAYERS; ++i) {
		// recieve signal
		int currsock = clisocks[i];
		if (recv(currsock, &sigtype, 1, 0) == -1) exitwerror("recvclisig", EXIT_ERRNO);

		switch (sigtype) {
			case CS_STD: recvvars(&dirs[i], clisocks[i]); break;		// standard signal, update variables
			case CS_COL: numcoll = numcoll == 0 ? 2 : i; break;		// collision occured, determine collision type
			default: exitwerror("recvclisig: invalid signal", EXIT_STD);
		}
	}
	return numcoll;
}

void recvvars(char* playerdir, int clisock) {
	char recvbuffer[CS_STDSIZE];
	if (recv(clisock, recvbuffer, CS_STDSIZE, 0) == -1) exitwerror("recvvars", EXIT_ERRNO);
	*playerdir = recvbuffer[PDIR];	// update player direction
}


void sendvars(int* socks, char* buffer) {
	char sigtype = SC_STD;
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) {
		if (send(socks[i], &sigtype, 1, 0) == -1) exitwerror("sendvars (sig)", EXIT_ERRNO);
		if (send(socks[i], buffer, SC_STDSIZE, 0) == -1) exitwerror("sendvars", EXIT_ERRNO);
	}
}

void endclients(int* clisocks, char winner) {
	char endsig = SC_END;
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) {
		int currsock = clisocks[i];
		if (send(currsock, &endsig, 1, 0) == -1) exitwerror("endclients: send", EXIT_ERRNO);	// send signal
		if (send(currsock, &winner, 1, 0) == -1) exitwerror("endclients: send", EXIT_ERRNO);	// send winner
	}
}

void exitwerror(const char* msg, enum EXIT_TYPE exittype) {
	switch (exittype) {
		case EXIT_STD:		fprintf(stderr, "%s\n", msg); break;
		case EXIT_ERRNO:	perror(msg); break;
	}
	exit(EXIT_FAILURE);
}

