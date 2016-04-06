#include "mainmenu.h"
#include <ncurses.h>
#include "visuals.h"

void displaymenu(void) {
	buildborder(MENUBORDER);

	const int strlen = 87;
	int maxy;
	int maxx;
	getmaxyx(stdscr, maxy, maxx);
	int midlocx = (maxx / 2) - (strlen / 2);

	// main title
	attron(COLOR_PAIR(MENUTITLE) | A_BOLD);
	move(3, midlocx);	printw("@@@@@@@@@@@@@@@           @                           @ @ @");
	move(4, midlocx);	printw("       @                  @                         @       @");
	move(5, midlocx);	printw("       @                  @                       @          @");
	move(6, midlocx);	printw("       @                  @                      @");
	move(7, midlocx);	printw("       @                  @                     @");
	move(8, midlocx);	printw("       @                  @                     @");
	move(9, midlocx);	printw("       @  @@   @@@ @   @  @      @ @@@ @ @ @@@  @              @   @   @@  @   @@@   @@");
	move(10, midlocx);	printw("       @  @ @  @ @ @@  @  @      @ @   @ @  @    @              @ @   @  @ @   @    @");
	move(11, midlocx);	printw("       @  @@   @ @ @ @ @  @      @ @ @ @@@  @     @          @   @   @     @   @@    @");
	move(12, midlocx);	printw("       @  @ @  @ @ @  @@  @      @ @ @ @ @  @       @       @    @   @   @ @   @      @");
	move(13, midlocx);	printw("       @  @  @ @@@ @   @  @@@@@@ @ @@@ @ @  @         @ @ @      @    @@@  @@@ @@@  @@");
	attroff(COLOR_PAIR(MENUTITLE) | A_BOLD);
	
	char playmsg[] = "PRESS [SPACE] TO BEGIN";
	char quitmsg[] = "PRESS Q TO QUIT";
	// press any key to begin
	move((maxy * 0.75f) - 1, (maxx / 2) - (sizeof (playmsg) / 2));
	attron(COLOR_PAIR(MENUPLAY) | A_BOLD);
	printw("%s", playmsg);
	attroff(COLOR_PAIR(MENUPLAY) | A_BOLD);
	
	// press q to quit
	attron(COLOR_PAIR(MENUQUIT) | A_BOLD);
	move((maxy * 0.75f) + 1, (maxx / 2) - (sizeof (quitmsg) / 2));
	printw("%s", quitmsg);
	attroff(COLOR_PAIR(MENUQUIT) | A_BOLD);
	
	refresh();
}

int getinput(void) {
	nodelay(stdscr, FALSE);
	int out = 0;
	int c = 0;
	while (c == 0) {
		c = getch() & A_CHARTEXT;
		switch (c) {
			case ' ':	out = PLAY; break;
			case 'q':	out = QUIT; break;
			default:	c = 0;
		}
	}
	return out;
}

