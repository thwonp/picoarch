#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "bench.h"
#include "main.h"
#include "options.h"
#include "plat.h"

/* Seconds left out of the summary: loading, and the sound handover when the
 * harness starts music after the game. */
#define WARMUP 5

static bool on;
static int secs, unlimited;
static FILE *log_f;
static uint64_t t0, sec_start, frame_start;
static unsigned cpu_last;
/* This second, then the run past WARMUP. */
static unsigned n, over, behind0, under0;
static uint64_t work_us, max_us;
static unsigned run_n, run_over, run_secs;
static uint64_t run_work_us, run_max_us, run_cpu;
static long run_hwm, run_min_avail = -1;
static unsigned run_behind0, run_under0;
/* Frame times past WARMUP in 0.5 ms steps, for percentiles: the single worst
 * frame is mostly when Muse got the CPU and does not repeat run to run. */
#define HIST_N 200
static unsigned hist[HIST_N];

/* The frame time in ms that fraction p of the run's frames are within. */
static double pct_ms(double p)
{
	unsigned total = 0, want, sum = 0, i;

	for (i = 0; i < HIST_N; i++)
		total += hist[i];
	want = (unsigned)(total * p);
	for (i = 0; i < HIST_N; i++)
		if ((sum += hist[i]) >= want && sum)
			return (i + 1) / 2.0;
	return HIST_N / 2.0;
}

static uint64_t clock_us(clockid_t c)
{
	struct timespec ts;

	clock_gettime(c, &ts);
	return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

/* Wall time. A frame's time is honest only with nothing blocking in it: game
 * sound closed (music playing) or unlimited. With game sound on it includes
 * the wait for room in the audio buffer, which paces the game. (Process CPU
 * time is no help: this kernel counts it in scheduler ticks.) */
static uint64_t now_us(void) { return clock_us(CLOCK_MONOTONIC); }

/* A "<key>: <n> kB" line of a /proc file, or -1. */
static long proc_kb(const char *path, const char *key)
{
	char line[128];
	size_t k = strlen(key);
	long v = -1;
	FILE *f = fopen(path, "r");

	if (!f) return -1;
	while (fgets(line, sizeof line, f))
		if (!strncmp(line, key, k) && line[k] == ':') {
			sscanf(line + k + 1, "%ld", &v);
			break;
		}
	fclose(f);
	return v;
}

bool bench_init(const char *core, const char *rom)
{
	FILE *f = fopen(BENCH_CONF, "r");

	if (!f) return false;
	if (fscanf(f, "%d %d", &secs, &unlimited) != 2 || secs <= 0) {
		fclose(f);
		return false;
	}
	fclose(f);
	if (!(log_f = fopen(BENCH_LOG, "a"))) return false;
	on = true;
	if (unlimited) {
		limit_frames = 0;
		enable_audio = 0;
	}
	fprintf(log_f, "run core=%s rom=%s secs=%d unlimited=%d frame_us=%d\n",
	        core, rom, secs, unlimited, plat_bench_frame_us());
	fprintf(log_f, "t frames over behind under max_ms avg_ms cpu rss hwm avail\n");
	t0 = sec_start = now_us();
	cpu_last = plat_cpu_ticks();
	behind0 = plat_bench_behind;
	under0 = plat_bench_underruns;
	return true;
}

bool bench_on(void) { return on; }

void bench_frame_start(void)
{
	if (on) frame_start = now_us();
}

static void second_done(uint64_t t)
{
	unsigned cpu = plat_cpu_ticks();
	unsigned s = (unsigned)((t - t0) / 1000000);
	unsigned behind = plat_bench_behind - behind0, under = plat_bench_underruns - under0;
	long rss = proc_kb("/proc/self/status", "VmRSS");
	long hwm = proc_kb("/proc/self/status", "VmHWM");
	long avail = proc_kb("/proc/meminfo", "MemAvailable");
	double el = (t - sec_start) / 1e6;

	fprintf(log_f, "%u %u %u %u %u %.1f %.2f %.0f %ld %ld %ld\n", s, n, over, behind, under,
	        max_us / 1000.0, n ? work_us / 1000.0 / n : 0, (cpu - cpu_last) / el, rss, hwm, avail);
	if (s > WARMUP) {
		if (!run_secs) {
			run_behind0 = behind0;
			run_under0 = under0;
		}
		run_secs++;
		run_n += n;
		run_over += over;
		run_work_us += work_us;
		run_cpu += cpu - cpu_last;
		if (max_us > run_max_us) run_max_us = max_us;
		if (hwm > run_hwm) run_hwm = hwm;
		if (avail >= 0 && (run_min_avail < 0 || avail < run_min_avail)) run_min_avail = avail;
	}
	cpu_last = cpu;
	behind0 = plat_bench_behind;
	under0 = plat_bench_underruns;
	n = over = 0;
	work_us = max_us = 0;
	sec_start = t;

	if ((int)s >= secs) {
		fprintf(log_f, "summary secs=%u fps=%.2f over=%u behind=%u under=%u max_ms=%.1f "
		        "p99_ms=%.1f p999_ms=%.1f avg_ms=%.2f cpu=%.1f hwm=%ld min_avail=%ld\n",
		        run_secs, run_secs ? (double)run_n / run_secs : 0, run_over,
		        plat_bench_behind - run_behind0, plat_bench_underruns - run_under0,
		        run_max_us / 1000.0, pct_ms(0.99), pct_ms(0.999),
		        run_n ? run_work_us / 1000.0 / run_n : 0,
		        run_secs ? (double)run_cpu / run_secs : 0, run_hwm, run_min_avail);
		fclose(log_f);
		log_f = NULL;
		on = false;
		should_quit = 1;
	}
}

void bench_frame_end(void)
{
	uint64_t t, d;

	if (!on) return;
	t = now_us();
	d = t - frame_start;
	n++;
	work_us += d;
	if (d > max_us) max_us = d;
	if (d > (uint64_t)plat_bench_frame_us()) over++;
	if (t - t0 > (WARMUP + 1) * 1000000ull)
		hist[d / 500 < HIST_N ? d / 500 : HIST_N - 1]++;
	if (t - sec_start >= 1000000)
		second_done(t);
}
