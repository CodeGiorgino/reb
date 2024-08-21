#!/usr/bin/env bash
set -e

info="\033[33;1mINFO\033[0m:"
error="\033[31;1mERROR\033[0m:"
usage="$info run ./install.sh --debug to compile with debug info"

if [ $(id -u) -eq 0 ]
then
    echo -e '$error cannot run as root'
    exit 1
fi

mkdir -p obj
mkdir -p build

mkdir -p ~/.config
mkdir -p ~/.config/reb
mkdir -p ~/.config/reb/models

cp assets/cpp.json ~/.config/reb/models/

GXX='g++'

if [ -z $1 ]
then
    CXX='-Wall -Wextra -std=c++23 -O2'
    echo -e "$info DEBUG: off"
elif [ $# -ne 1 ]
then
    echo -e "$error unexpected number of arguments"
    echo -e $usage
    exit 1
elif [ "$1" = '--debug' ]
then
    CXX='-Wall -Wextra -std=c++23 -Og -DDEBUG'
    echo -e "$info DEBUG: on"
else
    echo -e "$error unexpected argument provided"
    echo -e $usage
    exit 1
fi

for f in $(find 'src' -type f -path '*.cpp'); do
    base="$(basename "${f}")"
    dest="obj/${base%.*}.o"
    echo -e "$info building ${f}"
    $GXX $CXX -c -o "${dest}" "${f}"
done

$GXX \
    -o 'build/reb' \
    $(find 'obj' -type f -path '*.o') \
    'deps/cppjson.a'

if [ -L '/usr/local/bin/reb' ]
then
    echo -e "$error cannot create symlink /usr/local/bin/reb: a symlink with the same name already exists"
    exit 1
fi

echo -e "$info creating symlink /usr/local/bin/reb"
sudo ln -s "$(pwd)/build/reb" /usr/local/bin/reb
