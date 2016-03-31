#include "player.h"
#include <ncurses.h>

struct Player createpl(struct Point loc, Direction dir, char body) {
	struct Player p;
	p.loc = loc;
	p.dir = dir;
	p.body = body;
	return p;
}

void insertpl(const struct Player* p) {
	move(p->loc.y, p->loc.x);
	addch(p->body);
	refresh();
}

void erasepl(const struct Player* p) {
	move(p->loc.y, p->loc.x);
	delch();
	refresh();
}

void movepl(struct Player* p) {
	Direction pdir = p->dir;
	int dx = pdir == RIGHT ? 1 : (pdir == LEFT ? -1 : 0);
	int dy = pdir == DOWN ? 1 : (pdir == UP ? -1 : 0);
	
	p->loc.x += dx;
	p->loc.y += dy;
}

void checkdirchange(struct Player* p) {
	int dch = getch();
	switch (dch) {
		case KEY_UP:	p->dir = UP; break;
		case KEY_DOWN:	p->dir = DOWN; break;
		case KEY_LEFT:	p->dir = LEFT; break;
		case KEY_RIGHT:	p->dir = RIGHT; break;
		default:	p->dir = p->dir;	// no input received (if nonblocking)
	}
}

