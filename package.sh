#!/bin/bash

rm -rf pkg
rm -rf pkg.tar.xz pkg.tar.xz.sig

mkdir -p pkg/scenes/bin pkg/res

cp build/Opengl additional shader Lib README.md LICENSE pkg -r

rsync -r res/ pkg/res/ --exclude={'world','*.kra','*~'} 

cp scenes/bin/* pkg/scenes/bin -r

if [ $# -gt 0 ]
then
if [[ $1 == "-noarchive" ]]
then
exit
fi
fi

tar cf - pkg | xz -6 > pkg.tar.xz

read -p "Sign Data ?" answer
if [ "$answer" != "y" ] && [ "$answer" != "yes" ]
then
exit
fi

echo Please touch key
gpg --detach-sign pkg.tar.xz 