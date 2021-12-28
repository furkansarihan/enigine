#!/bin/bash

set -e
set -x

rm -rf build
mkdir build
pushd build

conan install .. --build=missing
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

bin/enigine
