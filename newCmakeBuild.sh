#!/bin/bash

#shell script to build and make cmake project
rm -rf build/
mkdir build/
cd build
cmake ..
make

