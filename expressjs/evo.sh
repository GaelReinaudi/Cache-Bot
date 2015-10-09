#!/bin/sh
pwd
echo $1
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./evoCache $1

