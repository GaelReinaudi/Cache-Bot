#!/bin/sh
pwd
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./evoCache $1
./extraCash $1

