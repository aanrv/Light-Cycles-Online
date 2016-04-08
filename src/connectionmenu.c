#include "connectionmenu.h"
#include <ncurses.h>
#include "visuals.h"

void showconnectionstatus(enum ConnectionStatus status) {
	clear();
	buildborder(MENUBORDER);

	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);

	if (status == CONNECTION_SUCCESS) {
		char connectmsg[]	= "Connected to server.";
		char waitingmsg[]	= "Waiting for other players...";

		move(maxy / 2 - 1, maxx / 2 - sizeof (connectmsg) / 2);
		printw("%s", connectmsg);
		move(maxy / 2 + 1, maxx / 2 - sizeof (waitingmsg) / 2);
		printw("%s", waitingmsg);
	} else {
		char connectionmsg[]	= "Unable to connect to server.";
		char ensuremsg[]	= "Make sure server is running and you are";
		char ensure2msg[]	= "connecting to the correct host and port.";
		char contmsg[]		= "Press [SPACE] to continue";

		int maxy;
		int maxx;
		getmaxyx(stdscr, maxy, maxx);

		move(maxy / 2 - 1, maxx / 2 - sizeof (connectionmsg) / 2);
		printw("%s", connectionmsg);
		move(maxy / 2 + 1, maxx / 2 - sizeof (ensuremsg) / 2);
		printw("%s", ensuremsg);
		move(maxy / 2 + 2, maxx / 2 - sizeof (ensure2msg) / 2);
		printw("%s", ensure2msg);

		move(maxy * 0.75f, maxx / 2 - sizeof (contmsg) / 2);
		printw("%s", contmsg);
	}
	refresh();
	
	nodelay(stdscr, FALSE);
	while (status == CONNECTION_FAIL && getch() != ' ')
		;
}

