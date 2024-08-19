#!/usr/bin/env bash
set -e

if [ $(id -u) -eq 0 ]
then
    echo -e '\033[31;1mERROR:\033[0m cannot run as root'
    exit 1
fi

mkdir -p obj
mkdir -p build

mkdir -p ~/.config
mkdir -p ~/.config/reb
mkdir -p ~/.config/reb/models

# curl -o ~/.config/reb/models/models.tar.gz https://raw.githubusercontent.com/CodeGiorgino/reb/master/assets/models.tar.gz
# tar -xvzf ~/.config/reb/models/models.tar.gz
cp assets/cpp.json ~/.config/reb/models/

GXX='g++'
CXX='-Wall -Wextra -std=c++23 -g'

for f in $(find 'src' -type f -path '*.cpp'); do
    base="$(basename "${f}")"
    dest="obj/${base%.*}.o"
    echo -e "\033[33;1mINFO:\033[0m building ${f}"
    $GXX $CXX -c -O2 -o "${dest}" "${f}"
done

$GXX \
    -o 'build/reb' \
    $(find 'obj' -type f -path '*.o') \
    'deps/cppjson.a'

if [ -L '/usr/local/bin/reb' ]
then
    echo -e '\033[31;1mERROR:\033[0m cannot create symlink /usr/local/bin/reb: a symlink with the same name already exists'
    exit 1
fi

echo -e '\033[33;1mINFO:\033[0m creating symlink /usr/local/bin/reb'
sudo ln -s "$(pwd)/build/reb" /usr/local/bin/reb
