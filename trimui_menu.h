#ifndef TRIMUI_MENU_H
#define TRIMUI_MENU_H

/* What the in-game menu asks for when it closes. */
enum { TRIMUI_MENU_GAME, TRIMUI_MENU_OPTIONS, TRIMUI_MENU_QUIT };

/* The in-game menu (plorpos-80b.8); runs until it closes. */
int trimui_menu(void);

#endif
