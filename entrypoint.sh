#!/bin/sh

cd /workspace
make O=$PWD -C /workspace/buildroot $@
