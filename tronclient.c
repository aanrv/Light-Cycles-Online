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

void exitwerror(const char* msg, int useserrno);

enum {APPNAME, PORTNUM, HOSTIP};

int main(int argc, char** argv) {
	if (argc < 2) { fprintf(stderr, "Usage: %s portnum [host ip]\n", argv[0]); exit(EXIT_FAILURE); }
	
	// create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) exitwerror("socket", 1);
	
	// create server address
	struct sockaddr_in seraddr;
	memset(&seraddr, 0, sizeof (struct sockaddr_in));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(atoi(argv[PORTNUM]));
	// if ip is provided, use it, otherwise assume same machine
	if(argc >= 3)
		if (inet_pton(AF_INET, argv[HOSTIP], &seraddr.sin_addr.s_addr) <= 0) exitwerror("Invalid ip.", 0);
	else
		seraddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	
	// connect to server
	int sersock = connect(sock, (struct sockaddr*) &seraddr, sizeof (struct sockaddr_in));
	if (sersock == -1) exitwerror("connect", 1);
	
	char tmpplayernum;
	if (recv(sock, &tmpplayernum, 1, 0) == -1) exitwerror("recv", 1);
	
	switch (tmpplayernum) {
		case PLAYER_1: puts("Connected as Player 1."); break;
		case PLAYER_2: puts("Connected as Player 2."); break;
		default: fprintf(stderr, "Received invalid player number.\n"); exit(EXIT_FAILURE);
	}
	
	close(sock);
	return EXIT_SUCCESS;
}

void exitwerror(const char* msg, int useserrno) {
	if (useserrno) perror(msg);
	else fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
