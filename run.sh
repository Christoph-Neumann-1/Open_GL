#! /bin/bash

#Runs the program, if it is located in the same directory. This is needed to set the LD_LIBRARY_PATH.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

mkdir $DIR/res/world

LD_LIBRARY_PATH="$DIR/Lib" $DIR/Opengl $DIR