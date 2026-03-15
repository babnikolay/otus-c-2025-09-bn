#!/bin/bash

rm -rf build/ dist/
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../dist
make
