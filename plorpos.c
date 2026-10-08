#include "plorpos.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "plat.h"

static bool playing;
static char title[128], artist[128];

static void read_now(void)
{
	char line[300], *t, *a;
	FILE *f = fopen(PLORPOS_NOW, "r");

	playing = false;
	title[0] = artist[0] = '\0';
	if (!f) return;
	if (fgets(line, sizeof line, f)) {
		line[strcspn(line, "\n")] = '\0';
		t = strchr(line, '\t');
		if (t) {
			*t++ = '\0';
			a = strchr(t, '\t');
			if (a) {
				*a++ = '\0';
				snprintf(artist, sizeof artist, "%s", a);
			}
			snprintf(title, sizeof title, "%s", t);
		}
		playing = !strcmp(line, "playing");
	}
	fclose(f);
}

void plorpos_init(void)
{
	read_now();
	plat_sound_set_quiet(playing);
}

void plorpos_tick(void)
{
	static unsigned next, frames, log_at;
	static int fpslog = -1;
	unsigned now = plat_get_ticks_ms();

	/* `now` is the wall clock in ms cut to 32 bits, so compared with an
	 * unset 0 it reads as the past half the time: start from it. */
	if (fpslog < 0) {
		fpslog = getenv("PLORPOS_FPSLOG") != NULL;
		next = now;
	}
	frames++;
	if (fpslog) {
		if (!log_at) log_at = now + 10000;
		if ((int)(now - log_at) >= 0) {
			fprintf(stderr, "plorpos: %.1f fps, music %s\n",
			        frames / ((now - log_at + 10000) / 1000.0), playing ? "on" : "off");
			frames = 0;
			log_at = now + 10000;
		}
	}
	plat_sound_quiet_pace();
	if ((int)(now - next) < 0) return;
	next = now + 500;
	read_now();
	/* Also when it has not changed: a reopen that failed - Muse still letting
	 * go of the codec, which it does 500 ms after it stops - goes again. */
	plat_sound_set_quiet(playing);
}

bool plorpos_refresh(void)
{
	bool was = playing;
	char t[sizeof title];

	snprintf(t, sizeof t, "%s", title);
	read_now();
	return was != playing || strcmp(t, title);
}

bool plorpos_music_playing(void) { return playing; }
const char *plorpos_title(void) { return title; }
const char *plorpos_artist(void) { return artist; }

bool plorpos_send(const char *cmd)
{
	char line[32];
	int fd = open(PLORPOS_CMD, O_WRONLY | O_NONBLOCK);
	bool ok;

	if (fd < 0) return false;
	snprintf(line, sizeof line, "%s\n", cmd);
	ok = write(fd, line, strlen(line)) == (ssize_t)strlen(line);
	close(fd);
	return ok;
}
