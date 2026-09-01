clear

read -rp "Compile GMLarcana for x86 Windows [Y/n]? " ANSWER

case "$ANSWER" in
    ""|Y|y|YES|Yes|yes)

        sh ./bump_patch.sh

        scons -c build-dir=build-win32 HOST_OS=win32 CC=i686-w64-mingw32-gcc CXX=i686-w64-mingw32-g++ architecture=x86

        scons -j7 build-dir=build-win32 HOST_OS=win32 CC=i686-w64-mingw32-gcc CXX=i686-w64-mingw32-g++ architecture=x86

        echo "Finished compiling."

        ;;

    *)

        echo "Compiler aborted."

        ;;

esac



