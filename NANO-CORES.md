# RG Nano cores: October 2026 update

All 14 libretro cores that [plorpOS nano](https://github.com/thwonp/plorpOS) runs on the Anbernic RG Nano, moved from FunKey-OS 2.3.0's stock builds to current upstream. Each one was benchmarked against the stock binary on the device and walked through every in-game menu option before it was accepted.

The release zip holds the cores under the file names plorpOS nano loads, plus `SOURCES.txt` with each core's upstream repository, exact commit, patches, licence and md5.

**Using them:** plorpOS nano loads cores from `/mnt/Libretro/cores` on the card before its own and FunKey-OS's, so copying a core there is enough to try it. They were built with the FunKey SDK 2.3.0 for the RG Nano (Cortex-A7, hard-float, NEON). They were tested only with the picoarch frontend from this fork's `plorpos-nano` branch.

**Building them:** `./build-nano-cores.sh <core>...` (pins in `Makefile`, patches in `patches/<core>/`), then `./package-nano-cores.sh <tag>`.

## How performance was measured

- One RG Nano, one picoarch frontend, the same games, with music playing in the background the whole time (plorpOS nano's normal use).
- **Speed** = unthrottled fps compared with FunKey-OS's stock binary, i.e. how far fast-forward can go and how much CPU is left for the music. Every core still runs at full speed (60 fps) at normal speed.
- Run-to-run noise is about ±2%, so anything within that is listed as "even".
- Acceptance gate: no more than −5% speed, +10% frame-time p99, +2 MB memory, and every menu option working. fceumm and fake08 were accepted below the speed gate on purpose (see their entries); the other 12 pass.

## Summary

| core | system | stock → new | speed vs stock |
|---|---|---|---|
| mednafen_lynx | Atari Lynx | efd1797 → fcdefcf | +2.4% Hard Drivin', +1.7% Blue Lightning |
| mednafen_ngp | Neo Geo Pocket / Color | 139fe34 → a50d5ac | −1.8% Metal Slug 2nd Mission, −3.5% KOF R-2 |
| mednafen_wswan | WonderSwan / Color | 2aeb47d → 4b01295 | +2.4% Final Fantasy IV, +1.8% Judgement Silversword |
| mednafen_supergrafx | PC Engine / SuperGrafx / CD | a776133 → 9c11dcd | +2.3% Air Zonk; CD games even |
| pokemini | Pokémon Mini | 1909580 → 132111b | −1.6% Party Mini, −3.2% Pinball Mini |
| snes9x2005 | SNES | 74d871d → a79dfe9 | −2.2% Star Fox; Yoshi's Island, Super Mario RPG even |
| fbalpha2012 | Arcade | 77167ce → 0ce3153 | even (1944, 19XX) |
| gambatte | Game Boy / Color | 13b7af7 → d9d6cd0 | Crystalis even, +4.1% Donkey Kong Land 2 |
| gpsp | Game Boy Advance | 143b0ab → 5819380 | +7.1% F-Zero Maximum Velocity; Golden Sun, Mario Kart even |
| picodrive | Genesis, 32X, Sega CD, SMS, GG, Pico | 3cd193ed → 1890c29 | even (Virtua Racing, Doom 32X, After Burner, Sonic CD) |
| pcsx_rearmed | PlayStation | 91362904 → c881679 | Crash even, −3.7% Ace Combat 2, −1.5% Parasite Eve |
| fceumm | NES / Famicom / FDS | 3544ff5 → 7a542da | −6.8% Battletoads, −4.5% Kirby's Adventure |
| fake08 | PICO-8 | cea6442 → 814991a | −7.7% Pico Racer, −11.5% Beckon the Hellspawn |
| mame2000 | Arcade (MAME 0.37b5) | 2ec60f6 → 181b42e | −0.7% Aliens, +1.4% Altered Beast |

Memory: every core within +1.7 MB of stock at peak (most within ±1 MB). Menu walk (every in-game option, including save and load states): no failures on any core.

## What's new, per core

### mednafen_lynx (Atari Lynx)
- No emulation changes upstream (build housekeeping).
- Fork fix: the in-game Options menu showed phantom entries; fixed.

### mednafen_ngp (Neo Geo Pocket / Color)
- Core options grouped into categories. No emulation changes.

### mednafen_wswan (WonderSwan / Color)
- WonderWitch flash emulation backported; flash and SRAM are saved.
- Serial port (link) emulation backported.
- Real-time clock rewrite backported, including a month fix.
- Cheats supported.
- Owner profile and language available as core options.
- Sound: HyperVoice right-channel fix; audio exact across save-state loads.
- Graphics: foreground window bound fix, out-of-bounds read fix.
- More deterministic save states.

### mednafen_supergrafx (PC Engine / SuperGrafx / PCE-CD)
- CHD disc images read through libretro-common's CHD reader.
- libretro-common resync.

### pokemini (Pokémon Mini)
- Save-state code moved to the current libretro API. No emulation changes.

### snes9x2005 (SNES)
- Code correctness fixes only.
- Fork fix: removed two in-game options the core doesn't have.

### fbalpha2012 (Arcade)
- zlib built in. No emulation changes.
- Fork fixes: "2-point" interpolation is actually selectable now (the old menu values didn't match the core), frameskip interval 0–9, removed an option the core doesn't have.
- Needs the FB Alpha 0.2.97.x-era romset, not FBNeo's.

### gambatte (Game Boy / Game Boy Color)
- New mapper: Sachen MMC1, including a save-state fix for it.
- Sound: noise-channel fix (the "Magnemite cry" and similar effects).
- Faster background palette path; integer-only audio resampler.
- Correctness, memory-safety and determinism audit of the libretro layer.
- Palette names updated; translations.
- Fork fix: palette lists regenerated from the core, so every palette is selectable.

### gpsp (Game Boy Advance)
- Save-type detection fix: an EEPROM signature found in a ROM no longer overrides real SRAM/flash saves.
- New option: real-time clock from the system clock.
- New option: sound output rate, now 32768 Hz by default (was 65536). That saves CPU and is where the F-Zero speedup comes from.
- Sound: 16-bit mixing, rounding fixes, no floating point in the audio path.
- GBA I/O read fixes, dynarec fix for code running from VRAM, several crash fixes.
- Graphics: mosaic fix on rotated/scaled backgrounds.
- Cheat fixes; Game Boy Player link state kept across save states.

### picodrive (Genesis / Mega Drive, 32X, Sega CD, Master System, Game Gear, Pico)
- 32X: synchronisation and interrupt-timing fixes, H32 mode handling (with an option to centre it), SH-2 fixes.
- Sega CD: fixed the wrong audio track playing after a seek.
- Master System: vertical scroll lock fix; FM sound state included in save states.
- Sega Pico supported; small Pico cartridges mirrored correctly.
- More memory exposed for cheats and achievements (CD Word RAM, 32X).
- Built with picodrive's ARM assembly CPU cores and SVP dynarec, as stock was. Without them, Virtua Racing ran at 47 fps instead of 118.

### pcsx_rearmed (PlayStation)
- CD-ROM behaviour corrected against real-hardware tests; CHD library updated.
- SPU (sound): envelope (ADSR) rewritten to current hardware documentation; better reverb-off handling.
- GTE (geometry) assembly reworked; native divide fix.
- M3U playlists accept absolute disc paths.
- Fork fix: removed in-game options for settings the core dropped.

### fceumm (NES / Famicom / FDS)
Accepted on purpose at −5 to −7% speed. There's no visible difference at normal speed (visible frame hitches: 1 vs 1 and 4 vs 1 per 45 s); fast-forward tops out around 1.65× instead of about 1.8×.
- Picture: PPU rewrite with fixes (e.g. Mega Man 3 sprites); MMC3 IRQ cleared on reset.
- Mappers: many new and corrected, mostly multicarts (620–625, 408, 343, 242, 438, 447, 511, 556, 598/599 and more).
- Sound: optional DMC pop reduction, optional ultrasonic-triangle mute, noise-channel power-on fix, FDS modulator fix, per-chip volume for expansion audio (VRC6, VRC7, N163, MMC5, Sunsoft 5B, FDS), sample-rate option.
- Built **without** HD texture pack support (`HAVE_HDPACK=0`): it's of no use on a 240×240 screen and cost about 3% speed.

### fake08 (PICO-8)
Accepted on purpose at −8 to −12% speed. There's no visible difference at normal speed, and fast-forward still reaches 2.1–2.6× realtime.
- Compatibility: newer PICO-8 syntax (`~` as xor, short `print`/`if`, `if…do`, `_ENV` fallbacks), chr/ord/peek/poke fixes (8192 values), negative colours, sprite transparency, sprite and screen mapped to user memory, custom fonts, more Lua functions (`stop`, `assert`, `select`, `tostring`), multicart fixes, vibrato fix.
- A Lua error returns to the menu instead of freezing.
- Save states (patches from plorpOS's TortOS port): the Lua heap is protected during a restore, and the Lua state is capped at PICO-8's 2 MB (big carts could overflow the old 1 MB buffer).

### mame2000 (Arcade, MAME 0.37b5)
- One frame less input lag: input is read before the frame runs.
- Analog stick input; the game pauses while the MAME menu is open.
- TMNT2 sprite fix; QSound audio reworked (Capcom QSound games).
- Rendering fast paths; a more robust core (re-entrant CPU loop, long-path and out-of-memory hardening).
- Build changes that bring it to stock speed:
  - the ARM assembly 68000/Z80 cores enabled, as in FunKey-OS's build;
  - compiled with the core's own Cortex-A7 flags;
  - audio kept at the configured 22050 Hz (`patches/mame2000/0005`). Upstream now mixes games that have a single FM chip at that chip's native rate (about 55.9 kHz for the YM2151), which is 2.5× the audio work per frame on the Nano;
  - stereo off by default, as on stock.
