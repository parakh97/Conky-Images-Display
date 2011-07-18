#!/bin/sh

[ -d build ] && {
    echo "WARNING: the directory 'build' already exists."
    echo "Your building environment might already been set up."
    echo "If not, please remove the 'build' directory and relaunch this script."
    exit 2
}

mkdir build && cd build || exit 1

cmake -DHAVE_COMPIZ=1 -DCMAKE_INSTALL_PREFIX=/usr ..

make -j$(grep -c processor /proc/cpuinfo)
