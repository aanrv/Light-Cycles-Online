#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>		// struct sockaddr_inA
#include <arpa/inet.h>		// inet_ntop()
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include "h.h"			// PLAYER_1, PLAYER_2

/**
 * Connects client socket to server with specified parameters.
 * If straddr is NULL, INADDR_LOOPBACK will be used,
 * otherwise straddr will be converted (using inet_ntop).
 * sersock and seraddr will be set to the server socket and server address respectively.
 * playernum will be set to either PLAYER_1 or PLAYER_2.
 */
void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, char* playernum);

/**
 * Exits program after printing msg.
 * If useserrno is set, perror will be used,
 * otherwise message is printed directly to stderr.
 */
void exitwerror(const char* msg, int useserrno);

enum {APPNAME, PORTNUM, HOSTIP};

int main(int argc, char** argv) {
	if (argc < 2) { fprintf(stderr, "Usage: %s portnum [host ip]\n", argv[0]); exit(EXIT_FAILURE); }

	// create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) exitwerror("socket", 1);

	int sersock;			// server socket
	struct sockaddr_in seraddr;	// server address
	char playernum;			// will hold PLAYER_1 or PLAYER_2 after connecting

	connecttoserver(sock, atoi(argv[PORTNUM]), argc >= 3 ? argv[HOSTIP] : NULL, &sersock, &seraddr, &playernum);

	close(sock);
	return EXIT_SUCCESS;
}

void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, char* playernum) {
	// initialize server address
	memset(seraddr, 0, sizeof (*seraddr));
	seraddr->sin_family = AF_INET;
	seraddr->sin_port = htons(port);
	if (straddr == NULL)
		seraddr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
		if (inet_pton(AF_INET, straddr, &(seraddr->sin_addr.s_addr)) <= 0) exitwerror("Invalid ip.", 0);

	// connect to server
	*sersock = connect(clisock, (struct sockaddr*) seraddr, sizeof (*seraddr));
	if (*sersock == -1) exitwerror("connect", 1);

	// recieve notification
	if (recv(clisock, playernum, 1, 0) == -1) exitwerror("recv", 1);

	switch (*playernum) {
		case PLAYER_1: puts("Connected as Player 1."); break;
		case PLAYER_2: puts("Connected as Player 2."); break;
		default: exitwerror("Received invalid player number.\n", 0);
	}
}

void exitwerror(const char* msg, int useserrno) {
	if (useserrno) perror(msg);
	else fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
