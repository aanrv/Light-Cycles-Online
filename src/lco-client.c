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
#include <ncurses.h>
#include "player.h"
#include "visuals.h"
#include "mainmenu.h"
#include "connectionmenu.h"
#include "instructionsmenu.h"
#include "gameovermenu.h"
#include "h.h"

enum {APPNAME, HOSTIP, PORTNUM};
unsigned char playernum;

/* Navigate main menu and sub menus until player decides to play. */
void loopmenu(void);

/* Initializes ncurses screen and starts the game's graphics. */
void playgame(int clisock);

/* Given arguments from main, assign address and port, both in host byte order. */
void assignaddrport(unsigned long* addr, unsigned short* port, int argc, char** argv);

/**
 * Connects client socket to server with specified parameters.
 * If straddr is NULL, INADDR_LOOPBACK will be used,
 * otherwise straddr will be converted (using inet_ntop).
 * sersock and seraddr will be set to the server socket and server address respectively.
 * playernum will be set to either PLAYER_1 or PLAYER_2.
 */
int connecttoserver(int clisock, unsigned short port, unsigned long addr, int* sersock, struct sockaddr_in* seraddr, unsigned char* playernum);

/* Recieve and return signal from server. */
char recvserversignal(int clisock);

/* Send variables to server. */
void sendtoserver(int clisock, const struct Player* player);

/* Send collision signal to server. */
void sendcol(int clisock);

/* Retrieve variables from server and update players accordingly. */
void updateplayers(int clisock, struct Player* players, int speed);

/* End ncurses screen, close socket if opened (-1 indicates not opened yet) and exit game. */
void quitgame(void);

/* End game due to collision. */
void endgame(int clisock);

int main(int argc, char** argv) {
	unsigned long clientaddr;
	unsigned short clientport;
	assignaddrport(&clientaddr, &clientport, argc, argv);
	
	int sock = socket(AF_INET, SOCK_STREAM, 0);		// client socket
	if (sock == -1) exitwerror("socket", EXIT_ERRNO);
	int sersock;						// server socket
	struct sockaddr_in seraddr;				// server address

	// main menu, wait for user to connect
	createcursesscreen();
	int connected;
	do {
		loopmenu();
		connected = connecttoserver(sock, clientport, clientaddr, &sersock, &seraddr, &playernum);
	} while (!connected);

	// both players connected, start game
	playgame(sock);

	return EXIT_SUCCESS;
}

void loopmenu(void) {
	int c;
	do {
		displaymenu();
		c = getinput();
		if (c == INSTRUCTIONS)	showinstructions();
		else if (c == QUIT)	quitgame();
	} while (c != PLAY);
}

void playgame(int clisock) {
	// getch() should be nonblocking
	if (nodelay(stdscr, TRUE) == ERR) exitwerror("Unable to set nodelay TRUE.", EXIT_STD);

	// get window boundaries
	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);				

	// create players with starting locations
	struct Point loc1 = {.x = maxx * 0.25f, .y = maxy / 2};
	struct Point loc2 = {.x = maxx * 0.75f, .y = maxy / 2};
	struct Player players[NUMPLAYERS];
	memset(players, 0, sizeof (struct Player) * NUMPLAYERS);

	players[PLAYER_1] = createpl(loc1, RIGHT, playerchar);
	players[PLAYER_2] = createpl(loc2, LEFT, playerchar);

	// place players on screen and display countdown before beginning
	clear();
	redrawplayers(players);
	displaycountdown(playernum, playernum == PLAYER_1 ? loc1 : loc2);
	buildborder(GAMEBORDER);

	for (EVER) {
		// RECIEVEING FROM SERVER
		char signal = recvserversignal(clisock);				// receive signal from server
		switch (signal) {
			case SC_STD: updateplayers(clisock, players, 1); break;		// standard signal, update variables and redraw
			case SC_END: endgame(clisock); break;				// end signal, end the game
			default: exitwerror("recv_server: invalid signal", EXIT_STD);
		}

		checkdirchange(&players[playernum]);					// if Player `plaernum`'s direction has changed, modify it

		// SENDING TO SERVER
		int collisionflag = willcollide(&players[playernum]);
		switch (collisionflag) {
			case 0:	sendtoserver(clisock, &players[playernum]); break;	// if no collision, send standard signal with variables to server
			case 1: sendcol(clisock); break;				// collision will/has occur(ed), send collision signal to server
			default: exitwerror("willcollide: invalid value", EXIT_STD);
		}

		usleep(refreshrate);
	}
}

void assignaddrport(unsigned long* addr, unsigned short* port, int argc, char** argv) {
	char* straddress = NULL;
	char* strport = NULL;
	
	int c;
	while ((c = getopt(argc, argv, "a:p:")) != -1) {
		switch (c) {
			case 'a':	straddress = optarg; break;
			case 'p':	strport = optarg; break;
		}
	}

	*port = strport == NULL ? DEFPORT : strtoport(strport);
	if (*port == 0) exitwerror("Invalid port.", EXIT_STD);

	if (straddress == NULL) {
		*addr = INADDR_LOOPBACK;
	} else {
		if (inet_pton(AF_INET, straddress, addr) <= 0) exitwerror("Invalid IP.", EXIT_STD);
		*addr = ntohl(*addr);
	}
}

int connecttoserver(int clisock, unsigned short port, unsigned long addr, int* sersock, struct sockaddr_in* seraddr, unsigned char* playernum) {
	// initialize server address
	memset(seraddr, 0, sizeof (*seraddr));
	seraddr->sin_family = AF_INET;
	seraddr->sin_port = htons(port);
	seraddr->sin_addr.s_addr = htonl(addr);

	// connect to server
	*sersock = connect(clisock, (struct sockaddr*) seraddr, sizeof (*seraddr));
	if (*sersock == -1) {
		showconnectionstatus(CONNECTION_FAIL);
		return 0;
	}

	showconnectionstatus(CONNECTION_SUCCESS);

	if (recv(clisock, playernum, 1, 0) == -1) exitwerror("recv", EXIT_ERRNO);
	return 1;
}

char recvserversignal(int clisock) {
	char sigtype;
	if (recv(clisock, &sigtype, 1, 0) == -1) exitwerror("recvsersig", EXIT_ERRNO);
	return sigtype;
}

void sendtoserver(int clisock, const struct Player* player) {
	char msgtype = CS_STD;
	char buffer[CS_STDSIZE];
	buffer[PDIR] = player->dir;

	if (send(clisock, &msgtype, 1, 0) == -1) exitwerror("send", EXIT_ERRNO);
	if (send(clisock, buffer, CS_STDSIZE, 0) == -1) exitwerror("send", EXIT_ERRNO);
}

void sendcol(int clisock) {
	char collisionsignal = CS_COL;
	if (send(clisock, &collisionsignal, 1, 0) == -1) exitwerror("sendcol: send", EXIT_ERRNO);
}

void updateplayers(int clisock, struct Player* players, int speed) {
	char buffer[SC_STDSIZE];
	if (recv(clisock, buffer, SC_STDSIZE, 0) == -1) exitwerror("recv", EXIT_ERRNO);

	int i;
	for (i = 0; i < speed; ++i) {
		// modify direction
		players[PLAYER_1].dir = buffer[P1DIR];
		players[PLAYER_2].dir = buffer[P2DIR];

		// modify location
		movepl(&players[PLAYER_1]);
		movepl(&players[PLAYER_2]);

		// redraw
		redrawplayers(players);
	}
}

void quitgame() {
	endwin();
	puts("Goodbye!");
	exit(EXIT_SUCCESS);
}

void endgame(int clisock) {
	// determine winning player
	char winner;
	if (recv(clisock, &winner, 1, 0) == -1) exitwerror("endgame: recv", EXIT_ERRNO);

	close(clisock);

	usleep(750000);
	showgameover(winner, playernum);

	endwin();
	exit(EXIT_SUCCESS);
}

void exitwerror(const char* msg, enum EXIT_TYPE exittype) {
	endwin();
	switch (exittype) {
		case EXIT_STD: 		fprintf(stderr, "%s\n", msg); break;
		case EXIT_ERRNO:	perror(msg); break;
	}
	exit(EXIT_FAILURE);
}

