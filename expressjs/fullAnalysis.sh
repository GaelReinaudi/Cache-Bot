#!/bin/sh
pwd
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./extraCash $1
./evoCache $1

