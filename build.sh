#!/usr/bin/env bash
set -xe

mkdir -p obj
mkdir -p build

GXX='g++'
CXX='-Wall -Wextra --std=c++23'
CC=''

for f in "$(find './src' -type f -path '*.cpp')"; do
    base="$(basename "${f}")"
    dest="./obj/${base%.*}.o"
    $GXX $CXX $CC -c -o "${dest}" "${f}"
done

$GXX $CXX $CC -o './build/reb' "$(find './obj' -type f -path '*.o')"
cp -r './assets' './build/'
