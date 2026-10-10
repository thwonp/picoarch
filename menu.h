#ifndef _MENU_H__
#define _MENU_H__

#include "config.h"
#include "libpicofe/menu.h"

int menu_init(void);
void menu_loop(void);
#ifdef TRIMUI
void menu_loop_options_only(void);
#endif
int menu_select_core(void);
int menu_select_content(char *filename, size_t len);
void menu_begin(void);
void menu_end(void);
void menu_finish(void);

#endif
