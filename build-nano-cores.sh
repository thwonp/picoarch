#!/bin/sh
# Builds libretro cores for the RG Nano (plorpos-ggv.39) with FunKey's SDK,
# each at the <core>_REVISION the Makefile pins, with patches/<core> applied.
# Run on the host: the clone and patches happen here (the tortos-toolchain
# container has no git or patch), the compile in the container.
#   ./build-nano-cores.sh <core>...        -> <core>_libretro.so, stripped
# A core already cloned is not cloned or patched again: `rm -rf <core>` to
# start it over at a new pin.
set -e
cd "$(dirname "$0")"
SDK=${SDK:-$(realpath ../TortOS/sdk-nano/sdk)}
X=/sdk/bin/arm-funkey-linux-musleabihf-
[ $# -gt 0 ] || { echo "usage: $0 <core>..." >&2; exit 1; }

sos=
for c; do
	make "$c" platform=funkey-s
	# picodrive's 68k core, Cyclone, is ARM asm written by a generator that
	# must run here: the container has no native compiler.
	[ "$c" = picodrive ] && [ ! -f picodrive/cpu/cyclone/Cyclone.s ] &&
		CC=gcc CXX=g++ CFLAGS=-O2 CXXFLAGS=-O2 LDFLAGS= \
			make -C picodrive/cpu/cyclone CONFIG_FILE=../cyclone_config.h
	sos="$sos ${c}_libretro.so"
done
# CXX too: the Makefile passes CXX=$(CXX) to every core and never sets it.
# LTO and section GC from the environment, which the cores' makefiles add to:
# FunKey-OS's stock cores were built so (gambatte at its stock commit comes
# out with the same .data to 136 bytes, and without it 60% larger), and the
# A/B against them measures the code, not the recipe.
F='-flto -fdata-sections -ffunction-sections'
docker run --rm -v "$PWD":/w -v "$SDK":/sdk:ro -w /w tortos-toolchain sh -c "
	export CFLAGS='$F' CXXFLAGS='$F' LDFLAGS='-flto -Wl,--gc-sections' &&
	make $sos platform=funkey-s CROSS_COMPILE=$X CXX=${X}g++ &&
	${X}strip $sos"
ls -l $sos
