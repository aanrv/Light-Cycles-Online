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
#include "h.h"			// PLAYER_1, PLAYER_2

typedef enum _errtype {NONE, STD, PERROR, NCURSE} errtype;
enum {APPNAME, PORTNUM, HOSTIP};

const char playerchar = 'A';
const int refreshrate = 50000;


/**
 * Connects client socket to server with specified parameters.
 * If straddr is NULL, INADDR_LOOPBACK will be used,
 * otherwise straddr will be converted (using inet_ntop).
 * sersock and seraddr will be set to the server socket and server address respectively.
 * playernum will be set to either PLAYER_1 or PLAYER_2.
 */
void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, char* playernum);

/**
 * Initializes ncurses screen and starts the game's graphics.
 */
void playgame();

/**
 * Set up window (disable cursor, disable line buffering, set nonblock, set keypad, etc.)
 */
errtype setscreen();

/**
 * Exits program after printing msg.
 * If useserrno is set, perror will be used,
 * otherwise message is printed directly to stderr.
 */
void exitwerror(const char* msg, errtype err);

int main(int argc, char** argv) {
	if (argc < 2) { fprintf(stderr, "Usage: %s portnum [host ip]\n", argv[0]); exit(EXIT_FAILURE); }

	// create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) exitwerror("socket", PERROR);

	int sersock;			// server socket
	struct sockaddr_in seraddr;	// server address
	char playernum;			// will hold PLAYER_1 or PLAYER_2 after connecting

	connecttoserver(sock, atoi(argv[PORTNUM]), argc >= 3 ? argv[HOSTIP] : NULL, &sersock, &seraddr, &playernum);
	
	playgame();
	
	//close(sock);
	return EXIT_SUCCESS;
}

void playgame() {
	int i;
	const int countdown = 3;
	puts("Starting...");
	for (i = countdown; i > 0; --i) { printf("%d ", i); fflush(stdout); sleep(1); }

	WINDOW* win = initscr();
	if (win == NULL) {
		fprintf(stderr, "Unable to initialize curses window.\n");
		exit(EXIT_FAILURE);
	}
	
	
	int maxy;
	int maxx;
	getmaxyx(win, maxy, maxx);
	
	cbreak();
	nodelay(win, 1);
	curs_set(0);
	keypad(win, 1);
	
	struct Point pl;
	pl.x = maxx / 2;
	pl.y = maxy / 2;
	struct Player p = createpl(pl, DOWN, playerchar);
	
	insertpl(&p);
	
	int withinscreen = 1;
	while (withinscreen) {
		checkdirchange(&p);	// check if key was pressed and change dir accordingly
		movepl(&p);		// move player to next position 
		usleep(refreshrate);
		withinscreen = withinbounds(&p, maxy, maxx);
	}
	
	nodelay(win, FALSE);
	getch();
	endwin();
}

errtype setscreen(WINDOW* win) {
	if (curs_set(0) == ERR) return ERR;		// hide cursor
	if (nodelay(win, TRUE) == ERR) return ERR;	// set nonblocking
	if (cbreak() == ERR) return ERR;		// disable line buffering
	if (keypad(win, 1) == ERR) return ERR;		// enable user terminal
	return NONE;
}

void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, char* playernum) {
	// initialize server address
	memset(seraddr, 0, sizeof (*seraddr));
	seraddr->sin_family = AF_INET;
	seraddr->sin_port = htons(port);
	if (straddr == NULL)
		seraddr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
		if (inet_pton(AF_INET, straddr, &(seraddr->sin_addr.s_addr)) <= 0) exitwerror("Invalid ip.", STD);

	// connect to server
	*sersock = connect(clisock, (struct sockaddr*) seraddr, sizeof (*seraddr));
	if (*sersock == -1) exitwerror("connect", PERROR);

	// recieve notification
	if (recv(clisock, playernum, 1, 0) == -1) exitwerror("recv", PERROR);

	switch (*playernum) {
		case PLAYER_1: puts("Connected as Player 1."); break;
		case PLAYER_2: puts("Connected as Player 2."); break;
		default: exitwerror("Received invalid player number", STD);
	}
}

void exitwerror(const char* msg, errtype err) {
	if (err == STD) fprintf(stderr, "%s\n", msg);
	else if (err == PERROR) perror(msg);
	else if (err == NCURSE) { endwin(); fprintf(stderr, "%s\n", msg); }
	exit(EXIT_FAILURE);
}

