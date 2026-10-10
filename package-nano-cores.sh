#!/bin/sh
# Packages the RG Nano cores built by build-nano-cores.sh for a release of
# this fork (plorpos-ggv.39): dist/picoarch-nano-cores-TAG.zip holding the
# cores under the file names plorpOS nano loads, NANO-CORES.md, and
# SOURCES.txt (each core's upstream repo, commit, patches and licence).
#   ./package-nano-cores.sh TAG
set -eu
cd "$(dirname "$0")"
TAG=$1
# <make name> <file name on the Nano> <licence>
CORES='mednafen_lynx mednafen_lynx_libretro.so GPL-2.0 (COPYING)
mednafen_ngp mednafen_ngp_libretro.so GPL-2.0 (COPYING)
mednafen_wswan mednafen_wswan_libretro.so GPL-2.0 (COPYING)
mednafen_supergrafx mednafen_supergrafx_libretro.so GPL-2.0 (COPYING)
pokemini pokemini_libretro.so GPL (LICENSE)
snes9x2005 snes9x2005_libretro.so Snes9x licence, non-commercial (copyright)
fbalpha2012 fbalpha2012_libretro.so FB Alpha licence, non-commercial (docs/license.txt)
gambatte gambatte_libretro.so GPL-2.0 (COPYING)
gpsp gpsp_libretro.so GPL-2.0 (COPYING)
picodrive picodrive_libretro.so PicoDrive licence, non-commercial (COPYING)
pcsx_rearmed pcsx_rearmed_libretro.so GPL-2.0 (COPYING)
fceumm fceumm_libretro.so GPL-2.0 (Copying)
fake-08 fake08_libretro.so MIT (LICENSE.MD)
mame2000 mame2000_libretro.so MAME licence, non-commercial (readme.txt)'

if [ -n "$(git status --porcelain --untracked-files=no -- . ':!libpicofe')" ]; then
	echo "uncommitted changes: SOURCES.txt would not match the cores" >&2
	exit 1
fi
FORK=$(git rev-parse HEAD)
OUT=dist/picoarch-nano-cores-$TAG
rm -rf "$OUT" "$OUT.zip"
mkdir -p "$OUT/cores"
cp NANO-CORES.md "$OUT/"
{
	echo "picoarch-nano-cores $TAG"
	echo
	echo "Built for the RG Nano (FunKey-OS 2.3.0, FunKey SDK 2.3.0) by build-nano-cores.sh"
	echo "in https://github.com/thwonp/picoarch at $FORK."
	echo "Each core's patches are patches/<core>/ in that tree; its build flags are"
	echo "<core>_FLAGS in the Makefile there."
	echo
	echo "$CORES" | while read -r c so lic; do
		[ -f "${c}_libretro.so" ] || { echo "no ${c}_libretro.so; run ./build-nano-cores.sh $c" >&2; exit 1; }
		cp "${c}_libretro.so" "$OUT/cores/$so"
		p=$(ls "patches/$c" 2>/dev/null | tr '\n' ' ')
		echo "$so"
		echo "  source:  $(git -C "$c" remote get-url origin) at $(git -C "$c" rev-parse HEAD)"
		echo "  patches: ${p:-none}"
		echo "  licence: $lic"
		echo "  md5:     $(md5sum "${c}_libretro.so" | cut -c1-32)"
	done
} > "$OUT/SOURCES.txt"
( cd dist && zip -qrX "picoarch-nano-cores-$TAG.zip" "picoarch-nano-cores-$TAG" )
echo "$OUT.zip ($(du -k "$OUT.zip" | cut -f1) kB, md5 $(md5sum "$OUT.zip" | cut -c1-8))"
