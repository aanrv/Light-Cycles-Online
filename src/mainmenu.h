#ifndef MAINMENU_H
#define MAINMENU_H

enum MenuOpts {PLAY, QUIT};

/* Display menu at start of game. */
void displaymenu(void);

/* Get user menu option. */
int getinput(void);

#endif // MAINMENU_H
