#!/bin/bash

rm -rf build bin

mkdir build

cd build

cmake ..

make

cd ..

./bin/file_system_shell