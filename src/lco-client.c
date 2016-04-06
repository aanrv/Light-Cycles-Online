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
#include "gameovermenu.h"
#include "h.h"

enum {APPNAME, PORTNUM, HOSTIP};
unsigned char playernum;

/**
 * Connects client socket to server with specified parameters.
 * If straddr is NULL, INADDR_LOOPBACK will be used,
 * otherwise straddr will be converted (using inet_ntop).
 * sersock and seraddr will be set to the server socket and server address respectively.
 * playernum will be set to either PLAYER_1 or PLAYER_2.
 */
void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, unsigned char* playernum);

/* Initializes ncurses screen and starts the game's graphics. */
void playgame(int clisock);

/* Recieve signal from server. */
void recv_server(int clisock, struct Player* players);

/* Retrieve variables from server and update accordingly. */
void updateplayers(int clisock, struct Player* players, int speed);

/* Exit game. */
void quitgame(void);

/* End game due to collision. */
void endgame(int clisock);

/* Send variables to server. */
void send_server(int clisock, struct Player* players);

/* Send collision signal to server. */
void sendcol(int clisock);

/* Create the curses screen. */
void createcursesscreen(void);

int main(int argc, char** argv) {
	if (argc < 2) { fprintf(stderr, "Usage: %s portnum [host ip]\n", argv[0]); exit(EXIT_FAILURE); }
	
	createcursesscreen();

	displaymenu();
	int c = getinput();
	
	if (c == QUIT) quitgame();

	// create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) exitwerror("socket", EXIT_ERRNO);

	int sersock;			// server socket
	struct sockaddr_in seraddr;	// server address

	connecttoserver(sock, atoi(argv[PORTNUM]), argc >= 3 ? argv[HOSTIP] : NULL, &sersock, &seraddr, &playernum);

	playgame(sock);

	//close(sock);
	return EXIT_SUCCESS;
}

void playgame(int clisock) {
	nodelay(stdscr, TRUE);
	
	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);

	// starting locations of players
	struct Point loc1 = {.x = maxx * 0.25f, .y = maxy / 2};
	struct Point loc2 = {.x = maxx * 0.75f, .y = maxy / 2};

	struct Player players[NUMPLAYERS];
	memset(players, 0, sizeof (struct Player) * NUMPLAYERS);
	players[PLAYER_1] = createpl(loc1, RIGHT, ACS_BLOCK);
	players[PLAYER_2] = createpl(loc2, LEFT, ACS_BLOCK);
	
	clear();
	redrawscreen(players);
	displaycountdown(playernum, playernum == PLAYER_1 ? loc1 : loc2);

	buildborder(GAMEBORDER);
	for (;;) {
		recv_server(clisock, players);				// receive signal from server
		checkdirchange(&players[playernum]);			// FIRST, check for a direction change

		if (!willcollide(&players[playernum])) {		// AFTER, make sure no collision will occur
			send_server(clisock, players);			// send standard message (variables, etc.) to server
		} else {
			sendcol(clisock);				// will collide
		}
		usleep(refreshrate);
	}
}

void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, unsigned char* playernum) {
	// initialize server address
	memset(seraddr, 0, sizeof (*seraddr));
	seraddr->sin_family = AF_INET;
	seraddr->sin_port = htons(port);
	if (straddr == NULL)
		seraddr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
		if (inet_pton(AF_INET, straddr, &(seraddr->sin_addr.s_addr)) <= 0) exitwerror("Invalid ip.", EXIT_STD);

	// connect to server
	*sersock = connect(clisock, (struct sockaddr*) seraddr, sizeof (*seraddr));
	if (*sersock == -1) exitwerror("Make sure the server is running and you are connecting to the correct port!\nconnect", EXIT_ERRNO);

	displayconnected();

	// receive notification
	if (recv(clisock, playernum, 1, 0) == -1) exitwerror("recv", EXIT_ERRNO);
}

void recv_server(int clisock, struct Player* players) {
	char sigtype;
	if (recv(clisock, &sigtype, 1, 0) == -1) exitwerror("recvsersig", EXIT_ERRNO);

	switch (sigtype) {
		case SC_STD: updateplayers(clisock, players, 1); break;
		case SC_END: endgame(clisock); break;
		default: exitwerror("recvsersig: invalid signal", EXIT_STD);
	}
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
		redrawscreen(players);
	}
}

void quitgame(void) {
	endwin();
	puts("\nGoodbye!");
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

void send_server(int clisock, struct Player* players) {
	char msgtype = CS_STD;
	char buffer[CS_STDSIZE];
	buffer[PDIR] = players[playernum].dir;

	if (send(clisock, &msgtype, 1, 0) == -1) exitwerror("send", EXIT_ERRNO);
	if (send(clisock, buffer, CS_STDSIZE, 0) == -1) exitwerror("send", EXIT_ERRNO);
}

void sendcol(int clisock) {
	char collisionsignal = CS_COL;
	if (send(clisock, &collisionsignal, 1, 0) == -1) exitwerror("sendcol: send", EXIT_ERRNO);
}

void exitwerror(const char* msg, int exittype) {
	endwin();
	switch (exittype) {
		case EXIT_STD: 		fprintf(stderr, "%s\n", msg);
		case EXIT_ERRNO:	perror(msg);
	}
	exit(EXIT_FAILURE);
}


unsigned short strtoport(char* str) {
	const int BASE = 10;		// conversion base
	long out = strtol(str, NULL, BASE);

	if (out < PORTMIN || out > PORTMAX) out = 0;
	return (unsigned short) out;
}

void createcursesscreen(void) {
	// initialize curses window and set screen options
	if (initscr() == NULL) exitwerror("Unable to initialize curses window.\n", EXIT_STD);
	if (!setscreen()) exitwerror("Unable to set screen settings.", EXIT_STD);	

	if (has_colors()) {
		start_color();
		assigncolors();
	}
}

