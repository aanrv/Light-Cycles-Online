#ifndef VISUALS_H
#define VISUALS_H

#include "player.h"

enum {MENUBORDER = 1, MENUTITLE, MENUPLAY, MENUQUIT, GAMEBORDER, P1COLOR, P2COLOR };	// color pairs
extern const int refreshrate;								// rate of redraw
extern const char playerchar;								// player body character

/* Create the curses screen. */
void createcursesscreen(void);

/* Set up window (disable cursor, disable line buffering, set nonblock, set keypad, etc.) */
int setscreen(void);

/* (Re)draw screen based on player locations and directions. */
void redrawplayers(struct Player* players);

/* Displays countdown before game begins. 
   loc indicates inital position (used for arrow). */
void displaycountdown(const char playernum, struct Point loc);

/* Create border around screen. */
void buildborder(int colorpair);

/* Create color pairs for players and border. */
void assigncolors(void);

#endif // VISUALS_H

