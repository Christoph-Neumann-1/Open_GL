#!/bin/bash

rm -rf pkg
rm pkg.tar.xz pkg.tar.xz.sig

mkdir -p pkg/scenes/bin pkg/modules/bin pkg/res

cp build/Opengl additional shader Lib README.md run.sh pkg -r

rsync -r res/ pkg/res/ --exclude={'res/world','*.kra','*~'} 

cp scenes/bin/* pkg/scenes/bin -r

cp modules/bin/* pkg/modules/bin -r

tar cf - pkg | xz -6 > pkg.tar.xz

echo Please touch key
gpg --detach-sign pkg.tar.xz 