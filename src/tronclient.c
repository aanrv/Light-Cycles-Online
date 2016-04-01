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

typedef enum _errtype {NONE, STD, PERROR} errtype;
enum {APPNAME, PORTNUM, HOSTIP};

const char playerchar = 'A';
const int refreshrate = 50000;		// rate of redraw

/**
 * Connects client socket to server with specified parameters.
 * If straddr is NULL, INADDR_LOOPBACK will be used,
 * otherwise straddr will be converted (using inet_ntop).
 * sersock and seraddr will be set to the server socket and server address respectively.
 * playernum will be set to either PLAYER_1 or PLAYER_2.
 */
void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, unsigned char* playernum);

/* Initializes ncurses screen and starts the game's graphics. */
void playgame(int clisock, int sersock, const unsigned char playernum);

/* Set up window (disable cursor, disable line buffering, set nonblock, set keypad, etc.) */
errtype setscreen();

/* (Re)draw screen based on player locations and directions. */
void redrawscreen(struct Player* players);

/* Recieve signal from server. */
void recvsersig(int clisock, struct Player* players);

/* Retrieve variables from server and update accordingly. */
void receivevariables(int clisock, struct Player* players);

/* Send variables to server. */
void sendvariables(int sersock, struct Player* players, const unsigned char playernum);

/* Checks if player is within given bounds. */
int withinbounds(const struct Player* p, int maxy, int maxx);

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
	unsigned char playernum;	// will hold PLAYER_1 or PLAYER_2 after connecting

	connecttoserver(sock, atoi(argv[PORTNUM]), argc >= 3 ? argv[HOSTIP] : NULL, &sersock, &seraddr, &playernum);

	playgame(sock, sersock, playernum);

	//close(sock);
	return EXIT_SUCCESS;
}

void playgame(int clisock, int sersock, const unsigned char playernum) {
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

	struct Point loc1;
	loc1.x = maxx * 0.75f;
	loc1.y = maxy / 2;

	struct Point loc2;
	loc2.x = maxx * 0.25f;
	loc2.y = maxy / 2;

	struct Player players[NUMPLAYERS];
	memset(players, 0, sizeof (struct Player) * NUMPLAYERS);
	players[PLAYER_1] = createpl(loc1, LEFT, playerchar);
	players[PLAYER_2] = createpl(loc2, RIGHT, playerchar);

	int withinscreen = 1;
	while (withinscreen) {
		recvsersig(clisock, players);				// receive variables from server
		for (i = 0; i < NUMPLAYERS; ++i) movepl(&players[i]);		// modify player locations

		redrawscreen(players);						// redraw screen based on variables
		withinscreen = withinbounds(&players[playernum], maxy, maxx);	// make sure current location is within bounds
		checkdirchange(&players[playernum]);				// check if key was pressed and modify directions accordingly

		sendvariables(clisock, players, playernum);			// send variables of direction to client to be read by server
		usleep(refreshrate);						// sleep
	}

	endwin();
}

void redrawscreen(struct Player* players) {
	clear();
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) insertpl(&players[i]);
	refresh();
}

errtype setscreen(WINDOW* win) {
	if (curs_set(0) == ERR) return ERR;		// hide cursor
	if (nodelay(win, TRUE) == ERR) return ERR;	// set nonblocking
	if (cbreak() == ERR) return ERR;		// disable line buffering
	if (keypad(win, 1) == ERR) return ERR;		// enable user terminal
	return NONE;
}

void connecttoserver(int clisock, unsigned short port, char* straddr, int* sersock, struct sockaddr_in* seraddr, unsigned char* playernum) {
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

	// receive notification
	if (recv(clisock, playernum, 1, 0) == -1) exitwerror("recv", PERROR);

	switch (*playernum) {
		case PLAYER_1: puts("Connected as Player 1."); break;
		case PLAYER_2: puts("Connected as Player 2."); break;
		default: exitwerror("Received invalid player number", STD);
	}
}

void recvsersig(int clisock, struct Player* players) {
	char sigtype;
	if (recv(clisock, &sigtype, 1, 0) == -1) exitwerror("recvsersig", PERROR);
	
	switch (sigtype) {
		case SC_STD: receivevariables(clisock, players); break;
		default: exitwerror("recvsersig: invalid signal", STD);
	}
}

void receivevariables(int clisock, struct Player* players) {
	char buffer[SC_STDSIZE];
	if (recv(clisock, buffer, SC_STDSIZE, 0) == -1) exitwerror("recv", PERROR);

	players[PLAYER_1].dir = buffer[P1DIR];
	players[PLAYER_2].dir = buffer[P2DIR];
}

void sendvariables(int clisock, struct Player* players, const unsigned char playernum) {
	char msgtype = CS_STD;
	char buffer[CS_STDSIZE];
	buffer[PDIR] = players[playernum].dir;
	
	if (send(clisock, &msgtype, 1, 0) == -1) exitwerror("send", PERROR);
	if (send(clisock, buffer, CS_STDSIZE, 0) == -1) exitwerror("send", PERROR);
}

void exitwerror(const char* msg, errtype err) {
	endwin();
	if (err == STD) fprintf(stderr, "%s\n", msg);
	else if (err == PERROR) perror(msg);
	exit(EXIT_FAILURE);
}


int withinbounds(const struct Player* p, int maxy, int maxx) {
	int out = 1;
	
	int cury = p->loc.y;
	int curx = p->loc.x;
	
	int dy = p->dir == DOWN ? 1 : (p->dir == UP ? -1 : 0);
	int dx = p->dir == RIGHT ? 1 : (p->dir == LEFT ? -1 : 0);
	
	cury += dy;
	curx += dx;
	
	if (cury >= maxy || cury < 0) out = 0;
	else if (curx >= maxx || curx < 0) out = 0;

	return out;
}

