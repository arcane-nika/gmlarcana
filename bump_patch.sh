#!/usr/bin/env bash
set -e

FILE="SConstruct"

# Read current patch version
PATCH=$(grep '^version_patch' "$FILE" | grep -oE '[0-9]+')

NEW_PATCH=$((PATCH + 1))

echo
echo "Patch version: $PATCH -> $NEW_PATCH"
read -rp "Apply change? [Y/n] " ANSWER

case "$ANSWER" in
    ""|Y|y|YES|Yes|yes)
        ;;
    *)
        echo "Cancelled."
        exit 0
        ;;
esac

sed -i -E \
    "s/^(version_patch[[:space:]]*=[[:space:]]*\")[0-9]+(\")/\1$NEW_PATCH\2/" \
    "$FILE"

echo "Done."
