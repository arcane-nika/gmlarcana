#!/usr/bin/env bash
set -u

if [ "$#" -ne 4 ]; then
    echo "usage: $0 <input-archive.a> <output-dir> <left-name> <right-name>"
    echo "example: $0 /tmp/interp-isolate-3/a12.a /tmp/interp-isolate-4 a121 a122"
    exit 1
fi

INPUT="$1"
OUTDIR="$2"
LEFTNAME="$3"
RIGHTNAME="$4"

mkdir -p "$OUTDIR"

mapfile -t MEMBERS < <(
    i686-w64-mingw32-ar t "$INPUT"
)

COUNT=${#MEMBERS[@]}
if [ "$COUNT" -lt 2 ]; then
    echo "archive has fewer than 2 members, cannot split"
    exit 1
fi

MID=$((COUNT / 2))

LEFT="$OUTDIR/$LEFTNAME.a"
RIGHT="$OUTDIR/$RIGHTNAME.a"

cp "$INPUT" "$LEFT"
cp "$INPUT" "$RIGHT"

# LEFT keeps first half, delete second half.
i686-w64-mingw32-ar d \
    "$LEFT" \
    "${MEMBERS[@]:$MID}"

# RIGHT keeps second half, delete first half.
i686-w64-mingw32-ar d \
    "$RIGHT" \
    "${MEMBERS[@]:0:$MID}"

i686-w64-mingw32-ranlib "$LEFT"
i686-w64-mingw32-ranlib "$RIGHT"

printf '%s\n' "${MEMBERS[@]:0:$MID}" > "$OUTDIR/$LEFTNAME.members.txt"
printf '%s\n' "${MEMBERS[@]:$MID}" > "$OUTDIR/$RIGHTNAME.members.txt"

echo "input archive: $INPUT"
echo "member count:  $COUNT"
echo "split point:   $MID / $((COUNT - MID))"
echo
echo "left archive:  $LEFT"
echo "left count:    $(i686-w64-mingw32-ar t "$LEFT" | wc -l)"
echo "left members file:  $OUTDIR/$LEFTNAME.members.txt"
echo
echo "right archive: $RIGHT"
echo "right count:   $(i686-w64-mingw32-ar t "$RIGHT" | wc -l)"
echo "right members file: $OUTDIR/$RIGHTNAME.members.txt"
