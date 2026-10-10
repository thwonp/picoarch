#!/bin/sh
# Builds picoarch for the TrimUI Model S (plorpOS, plorpos-80b) with the union
# trimui toolchain (gcc 6.1, arm926ej-s soft-float, glibc 2.23). Run inside the
# tortos-toolchain container with the toolchain at /sdk:
#   docker run --rm -v $PWD:/w -v <TortOS>/sdk-trimui/sdk:/sdk:ro -w /w tortos-toolchain ./build-trimui.sh
# Its host binaries want libmpfr.so.4, which /sdk/hostlib provides.
set -e
export LD_LIBRARY_PATH=/sdk/hostlib
# CFLAGS on the command line replaces the Makefile's -MMD, so nothing tracks
# headers or plat_sdl.c (included by plat_trimui.c): always build clean.
rm -f *.o funkey/*.o libpicofe/*.o libpicofe/linux/*.o picoarch
X=/sdk/usr/bin/arm-buildroot-linux-gnueabi-
S=/sdk/usr/arm-buildroot-linux-gnueabi/sysroot
SDLC="-I$S/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"
SDLL="-L$S/usr/lib -lSDL -lpthread"
make picoarch platform=trimui CROSS_COMPILE=$X \
	CFLAGS="$SDLC -mcpu=arm926ej-s -mtune=arm926ej-s -fno-PIC -DCONTENT_DIR='\"/mnt/SDCARD/Roms\"' -DPLORPOS -DTRIMUI -DPICO_HOME_DIR='\"/.picoarch/\"' -Ofast -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -fdata-sections -ffunction-sections -I./ -I./libretro-common/include/ -DREVISION=\\\"$(git rev-parse --short HEAD 2>/dev/null || echo trimui)\\\"" \
	LDFLAGS="$SDLL -lSDL_ttf -lc -ldl -lgcc -lm -lSDL -lasound -lpng -lz -Wl,--gc-sections -fno-PIC" \
	SDL_INCLUDES="$SDLC" SDL_LIBS="$SDLL"
${X}strip picoarch
