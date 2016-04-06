#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

/* Precondition for usage of file: initscr() has already been called. */

typedef enum _Direction {UP, DOWN, LEFT, RIGHT} Direction;

struct Point {
	int x;
	int y;
};

struct Player {
	struct Point loc;
	Direction dir;
	char body;
};

/* Creates and returns a struct Player. */
struct Player createpl(struct Point loc, Direction dir, char body);

/* Inserts player onto grid. */
void insertpl(const struct Player* p);

/* Clears player from grid. */
void clearpl(const struct Player* p);

/* Sets player values to next position in grid. */
void movepl(struct Player* p);

/* Checks if a key has been pressed for direction change. */
void checkdirchange(struct Player* p);

/* Checks if player will collide at next position. */
int willcollide(const struct Player* p);

#endif // PLAYER_H

