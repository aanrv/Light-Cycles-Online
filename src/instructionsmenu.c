#include "instructionsmenu.h"
#include <ncurses.h>
#include "visuals.h"

void showinstructions(void) {
	clear();
	buildborder(MENUBORDER);
	refresh();
	
	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);
	
	char controlmsg[]	= "Use the arrow keys to move.";
	char goalmsg[] 		= "Avoid hitting your opponent and the wall.";
	char winmsg[] 		= "First one to collide loses!";
	char contmsg[] 		= "Press [SPACE] to continue";
	
	move(maxy / 2 - 1, maxx / 2 - sizeof (controlmsg) / 2);
	printw("%s", controlmsg);
	move(maxy / 2, maxx / 2 - sizeof (goalmsg) / 2);
	printw("%s", goalmsg);
	move(maxy / 2 + 2, maxx / 2 - sizeof (winmsg) / 2);
	printw("%s", winmsg);
	move(maxy * 0.75f, maxx / 2 - sizeof (contmsg) / 2);
	printw("%s", contmsg);
	
	refresh();
	
	nodelay(stdscr, FALSE);
	while (getch() != ' ')
		;
	clear();
	nodelay(stdscr, TRUE);
}

