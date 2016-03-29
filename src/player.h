#ifndef PLAYER_H
#define PLAYER_H

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

/**
 * Creates and returns a struct Player.
 */
struct Player createpl(struct Point loc, Direction dir, char body);

/**
 * Inserts player onto grid.
 * Precondition: initscr() has already been called.
 */
void insertpl(const struct Player* p);

/**
 * Clears player from grid.
 * Precondition: initscr() has already been called.
 */
void clearpl(const struct Player* p);

/**
 * Moves player to next position on grid depending on direction.
 */
void movepl(struct Player* p);

/**
 * Checks if a key has been pressed for direction change.
 * Postcondition: p->dir may be modified.
 */
void checkdirchange(struct Player* p);

/**
 * Check to see if p is located within the provided bounds.
 */
int withinbounds(const struct Player* p, int maxy, int maxx);

#endif // PLAYER_H

