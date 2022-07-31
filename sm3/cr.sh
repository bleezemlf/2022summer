#!/bin/sh
echo  '\n-----------cmake begin----------\n'
cmake -S . -B cmake-build-release -D CMAKE_BUILD_TYPE=Release||exit
cd cmake-build-release||exit
echo  '\n-----------cmake end------------\n'

echo  '\n-----------make begin-----------\n'
make||exit
echo  '\n-----------make end-------------\n'

echo  '\n-----------program output:------\n'
./sm3||exit
echo  '\n-----------program end----------\n'
