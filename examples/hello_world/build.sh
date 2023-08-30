#!/bin/bash

set -e
set -x

BUILD_DIR="build"
CLEAN_BUILD=false

# Parse command line parameters
for arg in "$@"; do
  case $arg in
    --clean)
      CLEAN_BUILD=true
      ;;
  esac
done

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
  rm -rf $BUILD_DIR
  mkdir $BUILD_DIR
fi

# Create the build directory if it does not exist
if [ ! -d "$BUILD_DIR" ]; then
  mkdir $BUILD_DIR
fi

pushd $BUILD_DIR

conan install .. --build=missing
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

bin/hello_world

popd
