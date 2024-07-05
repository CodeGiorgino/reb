#!/usr/bin/env bash
set -e

if [ $(id -u) -eq 0 ]
then
    echo 'ERROR: cannot run as root'
    exit 1
fi

mkdir -p obj
mkdir -p build

mkdir -p ~/.config
mkdir -p ~/.config/reb
mkdir -p ~/.config/reb/models

# curl -o ~/.config/reb/models/models.tar.gz https://raw.githubusercontent.com/CodeGiorgino/reb/master/assets/models.tar.gz
# tar -xvzf ~/.config/reb/models/models.tar.gz
cp assets/cpp.config ~/.config/reb/models/

GXX='g++'
CXX='-Wall -Wextra -std=c++23'

for f in $(find 'src' -type f -path '*.cpp'); do
    base="$(basename "${f}")"
    dest="obj/${base%.*}.o"
    echo "INFO: building ${f}"
    $GXX $CXX -c -o "${dest}" "${f}"
done

$GXX $CXX \
    -o 'build/reb' \
    $(find 'obj' -type f -path '*.o')

if [ -L '/usr/local/bin/reb' ]
then
    echo 'ERROR: cannot create symlink /usr/local/bin/reb: symlink exists'
    exit 1
fi

echo 'INFO: creating symlink /usr/local/bin/reb'
sudo ln -s "$(pwd)/build/reb" /usr/local/bin/reb
