#ifndef VISUALS_H
#define VISUALS_H

#include "player.h"

enum {P1COLOR = 1, P2COLOR = 2, BORDERCOLOR = 3};	// colors
extern const int refreshrate;				// rate of redraw

/* Create color pairs for players and border. */
void assigncolors(void);

/* Create border around screen. */
void buildborder(void);

/* Set up window (disable cursor, disable line buffering, set nonblock, set keypad, etc.) */
int setscreen(void);

/* (Re)draw screen based on player locations and directions. */
void redrawscreen(struct Player* players);

#endif // VISUALS_H

