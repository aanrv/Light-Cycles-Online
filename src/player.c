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
	
	erasepl(p);
	
	p->loc.x += dx;
	p->loc.y += dy;
	
	insertpl(p);
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

