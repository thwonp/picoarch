#ifndef PLORPOS_H
#define PLORPOS_H
#include <stdbool.h>

/* plorpOS on the RG Nano (plorpos-ggv): music over gameplay.
 *
 * The launcher (nanoshelf) is Muse's one client and owns the play queue. It
 * writes what Muse is doing to PLORPOS_NOW, one line:
 *     <playing|paused|stopped>\t<title>\t<artist>
 * and reads commands from PLORPOS_CMD (a FIFO), one word a line: toggle,
 * next, prev. While music plays the game is quiet and its sound device is
 * closed - the Nano has one codec and no dmix, so Muse and the game take
 * turns on it, as plorpOS's H700 port does. */
#define PLORPOS_NOW "/tmp/plorpos/now"
#define PLORPOS_CMD "/tmp/plorpos/cmd"

/* Read the state once, before the sound device is first opened. */
void plorpos_init(void);
/* Every frame; looks at the state every 500 ms. */
void plorpos_tick(void);
/* Whether music is playing, as last read. */
bool plorpos_music_playing(void);
const char *plorpos_title(void);
const char *plorpos_artist(void);
/* Send one command word to the launcher; false when nobody is listening. */
bool plorpos_send(const char *cmd);

#endif
