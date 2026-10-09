#ifndef BENCH_H
#define BENCH_H
#include <stdbool.h>

/* Test-only speed and memory log for A/B core checks (plorpos-ggv.39). Off
 * unless BENCH_CONF exists. The launcher, not adb, starts PicoArch, so the
 * switches live in a file rather than the environment. One line:
 *     <seconds> <unlimited 0|1>
 * PicoArch then quits by itself after <seconds>, and with unlimited set runs
 * as fast as it can with no frameskip (the fast-forward path without its
 * frameskip, which would hide the core's real speed). Each run appends a
 * header, a line a second and a summary to BENCH_LOG. */
#define BENCH_CONF "/tmp/picoarch-bench.conf"
#define BENCH_LOG  "/tmp/picoarch-bench.log"

/* Read BENCH_CONF; true when benching. Before the first frame. */
bool bench_init(const char *core, const char *rom);
bool bench_on(void);
/* Each override in overrides/<core>.h against the options the core gives,
 * as "optcheck" lines in BENCH_LOG: an override replaces the core's values by
 * position, so a value upstream added or moved silently breaks the option. */
struct retro_core_option_definition;
struct retro_variable;
void bench_check_options(const struct retro_core_option_definition *defs);
void bench_check_variables(const struct retro_variable *vars);
/* Around core_run_frame, every frame. */
void bench_frame_start(void);
void bench_frame_end(void);

#endif
