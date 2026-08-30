#!/usr/bin/env bash
set -u

if [ "$#" -ne 2 ]; then
    echo "usage: $0 <archive.a> <output-name>"
    echo "example: $0 /tmp/interp-isolate-3/a11.a interp-a11"
    exit 1
fi

ARCHIVE="$1"
NAME="$2"

ROOT="$HOME/Coding/gmlarcana"
DIST="$HOME/.local/share/bottles/bottles/GameMaker-1.4/drive_c/users/annikamaries/Documents/GameMaker/Projects/REVERENGINE.gmx/dist-win32"

cd "$ROOT" || exit 1

echo "=== compiling $NAME ==="
echo "archive: $ARCHIVE"

i686-w64-mingw32-g++ \
  -O0 -g -m32 -fpic \
  -static-libgcc -static-libstdc++ \
  "$DIST/exception-test.cpp" \
  -o "$DIST/$NAME.exe" \
  -L/usr/i686-w64-mingw32/sys-root/mingw/lib \
  -Wl,--unresolved-symbols=ignore-all \
  -Wl,--noinhibit-exec \
  -Wl,--whole-archive \
    "$ARCHIVE" \
    build-win32/libgmla-project.a \
    build-win32/libgmla-asset.a \
    build-win32/libgmla-bytecode.a \
    build-win32/libgmla-beautify.a \
    build-win32/libgmla-ast.a \
    build-win32/libgmla-sys.a \
    build-win32/libgmla-common.a \
  -Wl,--no-whole-archive \
    build-win32/libsoloud.a \
  -lshlwapi -lcomdlg32 -lffi \
  -lSDL2 -lSDL2_ttf \
  -lglut32 -lopengl32 -lglu32 -lglew32

STATUS=$?

echo
if [ -f "$DIST/$NAME.exe" ]; then
    echo "=== EXE produced ==="
    ls -lh "$DIST/$NAME.exe"
else
    echo "=== NO EXE produced ==="
fi

exit $STATUS
