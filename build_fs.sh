#!/usr/bin/env bash

CC=arm-linux-gnueabihf-gcc
ARCH=armv7l

owd=$(pwd)
SCRIPT_DIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
cd "${SCRIPT_DIR}"

# build app
cd app
export CC
export ARCH
make
cd ..

# make fs-overlay
mkdir -p build/fs-overlay || exit 1
cd build/fs-overlay || exit 1
# copy fs
rsync -a "${SCRIPT_DIR}/fs/" .
# copy app
mkdir -p usr/bin
find "${SCRIPT_DIR}/app/build-$ARCH" -maxdepth 1 -type f -exec cp -t usr/bin {} +

cd "$owd"
