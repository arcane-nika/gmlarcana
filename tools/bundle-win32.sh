#!/bin/bash

set -e

EXE="build-win32/gmla.exe"
DIST="dist-win32"
MINGW_BIN="/usr/i686-w64-mingw32/sys-root/mingw/bin"

mkdir -p "$DIST"

declare -A seen

copy_deps()
{
    local file="$1"

    while read -r dll; do
        # Normalize DLL name for case-insensitive comparison.
        lower=$(echo "$dll" | tr '[:upper:]' '[:lower:]')

        # Windows system DLLs are supplied by Windows itself.
        case "$lower" in
            kernel32.dll|kernelbase.dll|ntdll.dll|msvcrt.dll|\
            user32.dll|gdi32.dll|advapi32.dll|shell32.dll|\
            shlwapi.dll|comdlg32.dll|ole32.dll|oleaut32.dll|\
            ws2_32.dll|opengl32.dll|glu32.dll|\
            winmm.dll|imm32.dll|version.dll|setupapi.dll|\
            rpcrt4.dll|secur32.dll|bcrypt.dll|crypt32.dll|\
            comctl32.dll|uxtheme.dll|dwmapi.dll|\
            d3d*.dll|dxgi.dll)
                continue
                ;;
        esac

        # Already processed?
        if [[ "${seen[$lower]}" == "1" ]]; then
            continue
        fi

        seen[$lower]=1

        # Find the MinGW version of this DLL.
        dep=$(find "$MINGW_BIN" -maxdepth 1 -type f \
            -iname "$dll" -print -quit)

        if [[ -z "$dep" ]]; then
            echo "WARNING: Could not find MinGW DLL: $dll"
            continue
        fi

        echo "Bundling: $(basename "$dep")"

        cp -u "$dep" "$DIST/"

        copy_deps "$dep"

    done < <(
        i686-w64-mingw32-objdump -p "$file" 2>/dev/null |
        sed -n 's/^[[:space:]]*DLL Name: //p'
    )
}

echo "Bundling dependencies for $EXE"
echo

copy_deps "$EXE"

echo
echo "Done."
