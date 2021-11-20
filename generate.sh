#!/bin/bash

rm -rf build bin my_disk

mkdir build

cd build

cmake ..

make

cd ..

./bin/file_system_shell