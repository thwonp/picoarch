#ifndef __PLAT_H__
#define __PLAT_H__

#include "libpicofe/plat.h"

struct audio_frame {
	int16_t left;
	int16_t right;
};

int  plat_init(void);
int  plat_reinit(void);
void plat_finish(void);
void plat_minimize(void);

void *plat_prepare_screenshot(int *w, int *h, int *bpp);
int plat_dump_screen(const char *filename);
int plat_load_screen(const char *filename, void *buf, size_t buf_size, int *w, int *h, int *bpp);

void plat_video_open(void);
void plat_video_set_msg(const char *new_msg, unsigned priority, unsigned msec);
void plat_video_process(const void *data, unsigned width, unsigned height, size_t pitch);
void plat_video_flip(void);
void plat_video_close(void);

unsigned plat_cpu_ticks(void);

int plat_sound_occupancy(void);
extern void (*plat_sound_write)(const struct audio_frame *data, int frames);
void plat_sound_resize_buffer(void);
#ifdef PLORPOS
#include <stdbool.h>
/* Close the sound device while music plays, and open it again after (see
 * plorpos.h); the frame timer paces the game meanwhile. */
void plat_sound_set_quiet(bool quiet);
void plat_sound_quiet_pace(void);
/* False while quiet: the core may skip making sound nobody hears
 * (RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE). */
bool plat_sound_audible(void);
/* For bench.c: frames the quiet pacer fell behind, sound callbacks the
 * buffer ran dry in, and the frame budget. */
extern unsigned plat_bench_behind, plat_bench_underruns;
int plat_bench_frame_us(void);
#endif

#endif /* __PLAT_H__ */
