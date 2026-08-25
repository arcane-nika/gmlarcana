#!/bin/bash
set -e

EXE="build-win32/gmla.exe"
OUT="dist-win32"
MINGW="/usr/i686-w64-mingw32"

rm -rf "$OUT"
mkdir -p "$OUT"

cp "$EXE" "$OUT/gmla.exe"

# Windows DLLs which are provided by the OS.
is_system_dll()
{
    case "${1,,}" in
        kernel32.dll|kernelbase.dll|ntdll.dll|user32.dll|gdi32.dll|gdi32full.dll|\
        advapi32.dll|shell32.dll|shlwapi.dll|ole32.dll|oleaut32.dll|\
        comdlg32.dll|combase.dll|ws2_32.dll|winmm.dll|imm32.dll|version.dll|\
        setupapi.dll|cfgmgr32.dll|rpcrt4.dll|secur32.dll|bcrypt.dll|\
        ucrtbase.dll|msvcrt.dll|opengl32.dll|glu32.dll|dwmapi.dll|\
        hid.dll|xinput1_4.dll)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

copy_deps()
{
    local file="$1"

    while read -r dll; do
        [ -z "$dll" ] && continue

        if is_system_dll "$dll"; then
            continue
        fi

        # Already copied
        if [ -f "$OUT/$dll" ]; then
            continue
        fi

        echo "Finding $dll..."

        local found
        found=$(find "$MINGW" -type f -iname "$dll" 2>/dev/null | head -n 1)

        if [ -z "$found" ]; then
            echo "WARNING: Could not find $dll"
            continue
        fi

        echo "  -> $found"
        cp "$found" "$OUT/$dll"

        # DLLs can have dependencies of their own.
        copy_deps "$OUT/$dll"
    done < <(
        i686-w64-mingw32-objdump -p "$file" |
        sed -n 's/^[[:space:]]*DLL Name: //p'
    )
}

copy_deps "$EXE"

echo
echo "Windows package:"
ls -lh "$OUT"
