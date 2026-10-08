#!/bin/sh
# Builds picoarch for the RG Nano exactly as FunKey-OS's rg_nano package does
# (FunKey/package/picoarch/picoarch.mk), with FunKey's SDK. Run inside the
# tortos-toolchain container with the SDK at /sdk:
#   docker run --rm -v $PWD:/w -v <TortOS>/sdk-nano/sdk:/sdk:ro -w /w tortos-toolchain ./build-nano.sh
set -e
# CFLAGS on the command line replaces the Makefile's -MMD, so nothing tracks
# headers or plat_sdl.c (included by plat_funkey.c): always build clean.
rm -f *.o funkey/*.o libpicofe/*.o libpicofe/linux/*.o picoarch
S=/sdk/arm-funkey-linux-musleabihf/sysroot
SDLC="-I$S/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT"
SDLL="-L$S/usr/lib -lSDL -lpthread"
make picoarch platform=funkey-s CROSS_COMPILE=/sdk/bin/arm-funkey-linux-musleabihf- \
	CFLAGS="$SDLC -DFUNKEY_S -DPLORPOS -Ofast -DNDEBUG -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -fdata-sections -ffunction-sections -flto -I./ -I./libretro-common/include/ -DREVISION=\\\"$(git rev-parse --short HEAD 2>/dev/null || echo nano)\\\"" \
	LDFLAGS="$SDLL -lc -ldl -lgcc -lm -lSDL -lasound -lpng -lz -Wl,--gc-sections -flto -lSDL_image -lSDL_ttf" \
	SDL_INCLUDES="$SDLC" SDL_LIBS="$SDLL"
