#!/bin/sh
pwd
echo "parameter 1............................."
echo $1
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./evoCache $1

