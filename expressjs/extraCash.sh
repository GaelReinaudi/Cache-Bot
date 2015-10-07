#!/bin/sh
pwd
echo $1
echo $2
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./extraCash $1 $2
