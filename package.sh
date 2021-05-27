#!/bin/bash

rm -rf pkg

mkdir -p pkg/scenes/bin pkg/modules/bin

cp build/Opengl shader Lib README.md run.sh pkg -r

rsync -r res/ pkg/res/ --exclude res/world

cp scenes/bin/* pkg/scenes/bin -r

cp modules/bin/* pkg/modules/bin -r

tar cf - pkg | xz -6 > pkg.tar.xz
