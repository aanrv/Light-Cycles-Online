#include "gameovermenu.h"
#include <ncurses.h>
#include <unistd.h>
#include "visuals.h"

void showgameover(const char winner, const unsigned char playernum) {
	clear();

	nodelay(stdscr, FALSE);
	buildborder(MENUBORDER);
	
	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);

	if (winner == 3) {
		move(maxy / 2, (maxx / 2) - 3);
		printw("DRAW!");
	} else {
		move(maxy / 2, (maxx / 2) - 5);
		if (winner == playernum) printw("YOU WIN!");
		else printw("YOU LOSE.");
	}
	
	char presskeymsg[] = "<PRESS [SPACE] TO EXIT>";
	move(maxy * 0.75f, (maxx / 2) - (sizeof (presskeymsg) / 2));
	printw("%s", presskeymsg);
	
	refresh();

	nodelay(stdscr, FALSE);
	while (getch() != ' ')
		;
}

