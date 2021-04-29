#!/bin/bash

mkdir -p pkg/scenes/bin pkg/modules/bin

cp build/Opengl Lib res README.md run.sh pkg -r

cp scenes/bin/* pkg/scenes/bin -r

cp modules/bin/* pkg/modules/bin -r
