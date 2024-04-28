#!/usr/bin/env bash
owd=$(pwd)
SCRIPT_DIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
cd "$SCRIPT_DIR"
echo "File system overlay:"
tree build/fs-overlay
rm build/fs-overlay -r
cd "$owd"
