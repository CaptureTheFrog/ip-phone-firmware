#!/usr/bin/env bash
owd=$(pwd)
cd "${0%/*}"
rm build/fs-overlay -r
cd "$owd"
