#!/bin/bash
#

make clean
make -j $(nproc)
cp pokefirered.gba "Pokemon - Fire Red nettux.gba"
