/* plorpOS on the TrimUI Model S (plorpos-80b.8): the in-game menu, one page
 * over the dimmed game in the shelf's look - Continue, Music, Save, Load,
 * Options (picoarch's own menu), Quit - as MinUI's is. The Music row asks
 * the shelf, which owns the queue (plorpos.h): A plays or pauses, left and
 * right change track. */
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core.h"
#include "main.h"
#include "plorpos.h"
#include "scale.h"
#include "trimui_menu.h"

#define FONT_SZ 15
#define TOP     22
#define BOT     18
#define ROW     26
#define SLOTS   9

enum { M_CONTINUE, M_MUSIC, M_SAVE, M_LOAD, M_OPTIONS, M_QUIT, M_COUNT };

static const SDL_Color C_TEXT = { 230, 230, 225, 0 }, C_DIM = { 140, 145, 150, 0 },
                       C_SEL = { 20, 20, 24, 0 }, C_ACC = { 120, 200, 160, 0 };
static SDL_Surface *screen;
static TTF_Font *font;
static uint16_t *bg;

/* The shelf's font, <app>/res/menu.ttf, picoarch being <app>/bin/picoarch. */
static bool font_open(void)
{
	char path[MAX_PATH];
	ssize_t n;
	char *s;

	if (font) return true;
	if ((n = readlink("/proc/self/exe", path, sizeof path - 32)) <= 0) return false;
	path[n] = '\0';
	if (!(s = strrchr(path, '/'))) return false;
	*s = '\0';
	if ((s = strrchr(path, '/'))) *s = '\0';
	strcat(path, "/res/menu.ttf");
	if (!TTF_WasInit() && TTF_Init() != 0) return false;
	if (!(font = TTF_OpenFont(path, FONT_SZ))) {
		PA_ERROR("menu: no font %s: %s\n", path, TTF_GetError());
		return false;
	}
	return true;
}

static void fill(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Rect rc = { (Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h };

	SDL_FillRect(screen, &rc, SDL_MapRGB(screen->format, r, g, b));
}

/* Text no wider than maxw, cut with "..."; align 0 left, 2 right. */
static void text(int x, int y, int maxw, const char *s, SDL_Color c, int align)
{
	char buf[256];
	int w = 0, h = 0;
	size_t n;
	SDL_Surface *t;
	SDL_Rect r;

	if (!s || !*s) return;
	snprintf(buf, sizeof buf, "%s", s);
	TTF_SizeUTF8(font, buf, &w, &h);
	n = strlen(buf);
	while (w > maxw && n > 3) {
		do n--; while (n > 0 && (buf[n] & 0xC0) == 0x80);
		memcpy(buf + n, "...", 4);
		TTF_SizeUTF8(font, buf, &w, &h);
	}
	if (!(t = TTF_RenderUTF8_Blended(font, buf, c))) return;
	r.x = (Sint16)(align == 2 ? x + maxw - t->w : x);
	r.y = (Sint16)y;
	SDL_BlitSurface(t, NULL, screen, &r);
	SDL_FreeSurface(t);
}

static bool row_enabled(int i)
{
	if (i == M_SAVE) return state_allowed();
	if (i == M_LOAD) return state_allowed() && state_exists(state_slot);
	return true;
}

static void draw(int sel)
{
	static const char *const names[M_COUNT] = { "Continue", "Music", "Save", "Load", "Options", "Quit" };
	const char *mark = plorpos_music_playing() ? ">" : plorpos_title()[0] ? "||" : "";
	char name[MAX_PATH], buf[300];
	int i, top = TOP + (SCREEN_HEIGHT - TOP - BOT - M_COUNT * ROW) / 2;
	const char *s;

	/* The game as it was, darkened to a quarter. */
	for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
		((uint16_t *)screen->pixels)[i] = (bg[i] >> 2) & 0x39E7;
	fill(0, 0, SCREEN_WIDTH, TOP, 32, 38, 44);
	s = strrchr(content->path, '/');
	snprintf(name, sizeof name, "%s", s ? s + 1 : content->path);
	if ((s = strrchr(name, '.'))) name[s - name] = '\0';
	text(6, 3, SCREEN_WIDTH - 12, name, C_TEXT, 0);

	for (i = 0; i < M_COUNT; i++) {
		int y = top + i * ROW;
		SDL_Color c = i == sel ? C_SEL : row_enabled(i) ? C_TEXT : C_DIM;

		if (i == sel) fill(0, y, SCREEN_WIDTH, ROW, 120, 200, 160);
		if (i == M_MUSIC && plorpos_title()[0]) {
			int w = 0, h;

			TTF_SizeUTF8(font, mark, &w, &h);
			text(8, y + 4, w, mark, i == sel ? C_SEL : C_ACC, 0);
			if (plorpos_artist()[0])
				snprintf(buf, sizeof buf, "%s - %s", plorpos_title(), plorpos_artist());
			else
				snprintf(buf, sizeof buf, "%s", plorpos_title());
			text(8 + w + 6, y + 4, SCREEN_WIDTH - 22 - w, buf, c, 0);
			continue;
		}
		text(8, y + 4, 140, i == M_MUSIC ? "Music: nothing queued" : names[i], c, 0);
		if (i == M_SAVE || i == M_LOAD) {
			snprintf(buf, sizeof buf, "<  Slot %d%s  >", state_slot + 1,
			         i == M_LOAD && !state_exists(state_slot) ? " (empty)" : "");
			text(150, y + 4, SCREEN_WIDTH - 158, buf, c, 2);
		}
	}

	fill(0, SCREEN_HEIGHT - BOT, SCREEN_WIDTH, BOT, 32, 38, 44);
	text(6, SCREEN_HEIGHT - BOT + 1, SCREEN_WIDTH - 12,
	     sel == M_MUSIC ? "A play/pause   < > track   B back" :
	     sel == M_SAVE || sel == M_LOAD ? "A select   < > slot   B back" : "A select   B back",
	     C_DIM, 0);
	SDL_Flip(screen);
}

/* Closed while a key is held, the game would get it: wait for its release. */
static void wait_release(SDLKey k)
{
	SDL_Event e;
	Uint32 until = SDL_GetTicks() + 2000;

	SDL_PumpEvents();
	if (!SDL_GetKeyState(NULL)[k]) return;
	while (SDL_GetTicks() < until) {
		while (SDL_PollEvent(&e))
			if (e.type == SDL_KEYUP && e.key.keysym.sym == k) return;
		SDL_Delay(10);
	}
}

int trimui_menu(void)
{
	int sel = M_CONTINUE;   /* every time, as on MinUI */
	int ret = TRIMUI_MENU_GAME, delay, interval, redraws = 0;
	Uint32 poll = 0, pressed = 0, sum = 0, worst = 0;
	bool done = false;
	SDL_Event e;

	screen = SDL_GetVideoSurface();
	if (!font_open() || !(bg || (bg = malloc(SCREEN_PITCH * SCREEN_HEIGHT))))
		return TRIMUI_MENU_OPTIONS;
	memcpy(bg, screen->pixels, SCREEN_PITCH * SCREEN_HEIGHT);
	plorpos_refresh();
	SDL_GetKeyRepeat(&delay, &interval);
	SDL_EnableKeyRepeat(300, 60);
	wait_release(SDLK_ESCAPE);
	draw(sel);

	while (!done) {
		bool redraw = false;

		while (!done && SDL_PollEvent(&e)) {
			SDLKey k;

			if (e.type == SDL_QUIT) { ret = TRIMUI_MENU_QUIT; done = true; break; }
			if (e.type != SDL_KEYDOWN) continue;
			k = e.key.keysym.sym;
			pressed = SDL_GetTicks();
			redraw = true;
			switch (k) {
			case SDLK_UP:   sel = (sel + M_COUNT - 1) % M_COUNT; break;
			case SDLK_DOWN: sel = (sel + 1) % M_COUNT; break;
			case SDLK_LEFT: case SDLK_RIGHT: case SDLK_TAB: case SDLK_BACKSPACE: {
				int d = k == SDLK_LEFT || k == SDLK_TAB ? -1 : 1;

				if (sel == M_MUSIC) {
					plorpos_send(d < 0 ? "prev" : "next");
					poll = SDL_GetTicks() - 250;   /* look again soon */
				} else if (sel == M_SAVE || sel == M_LOAD) {
					state_slot = (state_slot + SLOTS + d) % SLOTS;
				}
				break;
			}
			case SDLK_SPACE:   /* A */
				if (!row_enabled(sel)) break;
				switch (sel) {
				case M_MUSIC:
					plorpos_send("toggle");
					poll = SDL_GetTicks() - 250;
					break;
				case M_SAVE:    state_write(); done = true; break;
				case M_LOAD:    state_read(); done = true; break;
				case M_OPTIONS: ret = TRIMUI_MENU_OPTIONS; done = true; break;
				case M_QUIT:    ret = TRIMUI_MENU_QUIT; done = true; break;
				default:        done = true; break;
				}
				if (done) wait_release(k);
				break;
			case SDLK_LCTRL:   /* B */
			case SDLK_ESCAPE:  /* MENU */
				done = true;
				wait_release(k);
				break;
			default:
				redraw = false;
				break;
			}
		}
		if (done) break;
		/* What plays can change under the menu - a track ends, or a press
		 * above was answered: look again every 400 ms. */
		if (SDL_GetTicks() - poll >= 400) {
			poll = SDL_GetTicks();
			if (plorpos_refresh()) redraw = true;
		}
		if (redraw) {
			draw(sel);
			if (pressed) {
				Uint32 ms = SDL_GetTicks() - pressed;

				sum += ms;
				if (ms > worst) worst = ms;
				redraws++;
				pressed = 0;
			}
		}
		SDL_Delay(16);
	}

	SDL_EnableKeyRepeat(delay, interval);
	/* Options draws over the game, the game over nothing. */
	if (ret == TRIMUI_MENU_OPTIONS)
		memcpy(screen->pixels, bg, SCREEN_PITCH * SCREEN_HEIGHT);
	else
		memset(screen->pixels, 0, SCREEN_PITCH * SCREEN_HEIGHT);
	SDL_Flip(screen);
	if (redraws)
		fprintf(stderr, "plorpos menu: %d redraws after input, %u ms average, %u ms worst\n",
		        redraws, (unsigned)(sum / redraws), (unsigned)worst);
	return ret;
}
