#include "visuals.h"
#include <ncurses.h>
#include "player.h"
#include "h.h"

const int refreshrate = 10000;

void assigncolors(void) {
	if (can_change_color()) {
		init_color(COLOR_GREEN, 51, 255, 51);
		init_color(COLOR_BLUE, 51, 255, 255);
		init_color(COLOR_RED, 255, 51, 51);
	}

	init_pair(P1COLOR, COLOR_GREEN, COLOR_GREEN);
	init_pair(P2COLOR, COLOR_BLUE, COLOR_BLUE);
	init_pair(BORDERCOLOR, COLOR_RED, COLOR_BLACK);
}

void buildborder(void) {
	int border_attr = COLOR_PAIR(BORDERCOLOR) | A_BOLD;
	if (has_colors()) attron(border_attr);
	border(ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
	if (has_colors()) attroff(border_attr);
	refresh();
}

void redrawscreen(struct Player* players) {
	int i;
	for (i = 0; i < NUMPLAYERS; ++i) {
		int currcolor;
		switch (i + 1) {
			case P1COLOR:	currcolor = P1COLOR; break;
			case P2COLOR:	currcolor = P2COLOR; break;
			default:	exitwerror("redrawscreen: Invalid color.", EXIT_STD);
		}
		int attributes = COLOR_PAIR(currcolor) | A_BOLD | A_INVIS;
		if (has_colors()) attron(attributes);
		insertpl(&players[i]);
		if (has_colors()) attroff(attributes);
	}
	refresh();
}

int setscreen(void) {
	if (curs_set(0) == ERR) return 0;		// hide cursor
	if (nodelay(stdscr, TRUE) == ERR) return 0;	// set nonblocking
	if (cbreak() == ERR) return 0;			// disable line buffering
	if (keypad(stdscr, 1) == ERR) return 0;		// enable user terminal
	return 1;
}

