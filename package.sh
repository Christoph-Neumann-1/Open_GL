#!/bin/bash

rm -rf pkg
rm -rf pkg.tar.xz pkg.tar.xz.sig

mkdir -p pkg/scenes/bin pkg/modules/bin pkg/res

cp build/Opengl additional shader Lib README.md run.sh pkg -r

rsync -r res/ pkg/res/ --exclude={'world','*.kra','*~'} 

cp scenes/bin/* pkg/scenes/bin -r

cp modules/bin/* pkg/modules/bin -r
if [ $# -gt 0 ]
then
if [[ $1 == "-noarchive" ]]
then
exit
fi
fi

tar cf - pkg | xz -6 > pkg.tar.xz

if [ $# -gt 0 ] && [[ $1 != "-y" ]]
then
read -p "Sign Data ?" answer
if [ "$answer" != "y" ] && [ "$answer" != "yes" ]
then
exit
fi
fi

echo Please touch key
gpg --detach-sign pkg.tar.xz 