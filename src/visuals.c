#include "visuals.h"
#include <ncurses.h>
#include <unistd.h>
#include "player.h"
#include "h.h"

const int refreshrate = 70000;
const char playerchar = '0';

void createcursesscreen(void) {
	// initialize curses window and set screen options
	if (initscr() == NULL) exitwerror("Unable to initialize curses window.\n", EXIT_STD);
	if (!setscreen()) exitwerror("Unable to set screen settings.", EXIT_STD);	

	if (has_colors()) {
		start_color();
		assigncolors();
	}
}

int setscreen(void) {
	const int screenlen	= 30;
	const int screenheight	= 100;
	if (wresize(stdscr, screenlen, screenheight)	== ERR) return 0;	// set window size
	if (nodelay(stdscr, TRUE)			== ERR) return 0;	// don't block getch
	if (curs_set(FALSE)				== ERR) return 0;	// hide cursor
	if (cbreak()					== ERR) return 0;	// disable line buffering
	if (keypad(stdscr, TRUE)			== ERR) return 0;	// enable user terminal
	if (noecho()					== ERR) return 0;	// keyboard inputs shouldn't be displayed
	return 1;
}

void redrawplayers(struct Player* players) {
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) {
		int currcolor;
		switch (i) {
			case PLAYER_1:	currcolor = P1COLOR; break;
			case PLAYER_2:	currcolor = P2COLOR; break;
		}
		int attributes = COLOR_PAIR(currcolor) | A_BOLD;
		if (has_colors()) attron(attributes);
		insertpl(&players[i]);
		if (has_colors()) attroff(attributes);
	}
	refresh();
}

void displaycountdown(const char playernum, struct Point loc) {
	buildborder(GAMEBORDER);
	
	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);
	
	const int strsize = 17;
	move(maxy * 0.25f, (maxx / 2) - (strsize / 2));
	printw("YOU ARE PLAYER %d!", playernum + 1);
	
	// show arrow indicating player location
	move(loc.y + 1, loc.x);
	attron(COLOR_PAIR(MENUPLAY) | A_BOLD);
	addch(ACS_UARROW);
	attroff(COLOR_PAIR(MENUPLAY) | A_BOLD);

	refresh();
	sleep(1);

	int countdown = 3;
	do {
		move(maxy / 2, maxx / 2);	// move to center of screen
		printw("%d", countdown);
		refresh();
		sleep(1);
	} while (--countdown);
	
	clear();
	refresh();
}

void buildborder(int colorpair) {
	int border_attr = COLOR_PAIR(colorpair) | A_BOLD | A_INVIS;
	if (has_colors()) attron(border_attr);

	border(ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);

	if (has_colors()) attroff(border_attr);
	refresh();
}

void assigncolors(void) {
	init_pair(MENUBORDER, COLOR_GREEN, COLOR_GREEN);
	init_pair(MENUTITLE, COLOR_WHITE, COLOR_BLACK);
	init_pair(MENUPLAY, COLOR_WHITE, COLOR_BLACK);
	init_pair(MENUQUIT, COLOR_WHITE, COLOR_BLACK);
	init_pair(GAMEBORDER, COLOR_RED, COLOR_RED);
	init_pair(P1COLOR, COLOR_CYAN, COLOR_BLACK);
	init_pair(P2COLOR, COLOR_MAGENTA, COLOR_BLACK);
}

