rm -rf build
sh ./bump_patch.sh
scons -c
scons -Q arch=x86 2>&1 | tee build.log
echo "--- FINISHED COMPILING ---"
