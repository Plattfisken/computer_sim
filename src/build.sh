#!/bin/bash
PROGRAM_NAME="Program";
SOURCE_FILE_NAME="unity_build_root.c";

LIBS=""
# for raylib:
LIBS="$(pkg-config --libs --cflags raylib)"

export ASAN_OPTIONS=detect_odr_violation=0
WARNINGS="-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -fsanitize=address"
FLAGS="-g3 -O0 $WARNINGS"

mkdir -p ../build;
clang ui_layout.c $FLAGS $LIBS --shared -fPIC -o ../build/ui_layout.so
clang $SOURCE_FILE_NAME $LIBS $FLAGS -o ../build/"$PROGRAM_NAME" && echo "$PROGRAM_NAME"; # echo program name so it can be used by run.sh
