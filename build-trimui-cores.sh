#!/bin/sh
# Builds libretro cores for the TrimUI Model S (plorpOS, plorpos-80b) with the
# union trimui toolchain, each at the <core>_REVISION the Makefile pins, with
# patches/<core> applied. As build-nano-cores.sh: the clone and patches on the
# host, the compile in the tortos-toolchain container. Run it in a checkout of
# its own (a worktree), since the cores' trees are shared with the Nano's:
#   ./build-trimui-cores.sh <core>...        -> <core>_libretro.so, stripped
set -e
cd "$(dirname "$0")"
SDK=${SDK:-$(realpath ../TortOS/sdk-trimui/sdk)}
X=/sdk/usr/bin/arm-buildroot-linux-gnueabi-
[ $# -gt 0 ] || { echo "usage: $0 <core>..." >&2; exit 1; }

sos=
for c; do
	make "$c" platform=trimui
	[ "$c" = picodrive ] && [ ! -f picodrive/cpu/cyclone/Cyclone.s ] &&
		CC=gcc CXX=g++ CFLAGS=-O2 CXXFLAGS=-O2 LDFLAGS= \
			make -C picodrive/cpu/cyclone CONFIG_FILE=../cyclone_config.h HAVE_ARMv6=0
	sos="$sos ${c}_libretro.so"
done
docker run --rm -e LD_LIBRARY_PATH=/sdk/hostlib -v "$PWD":/w -v "$SDK":/sdk:ro -w /w tortos-toolchain sh -c "
	export CFLAGS='-fdata-sections -ffunction-sections' LDFLAGS='-Wl,--gc-sections' &&
	make $sos platform=trimui CROSS_COMPILE=$X CXX=${X}g++ &&
	${X}strip $sos"
ls -l $sos
