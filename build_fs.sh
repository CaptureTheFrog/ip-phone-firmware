#!/usr/bin/env bash
owd=$(pwd)
cd "${0%/*}"
mkdir -p build/fs-overlay || exit 1
cd build/fs-overlay || exit 1
mkdir root
touch root/testfile
cd "$owd"
