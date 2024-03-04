#!/bin/bash

set -e
set -x

BUILD_DIR="build"
CLEAN_BUILD=false
BUILD_TYPE="Debug"  # Default build type is Debug
RUN_AFTER_BUILD=false  # Default is not to run

# Parse command line parameters
for arg in "$@"; do
  case $arg in
    --clean)
      CLEAN_BUILD=true
      ;;
    --release)
      BUILD_TYPE="Release"
      ;;
    --debug)
      BUILD_TYPE="Debug"
      ;;
    --run)
      RUN_AFTER_BUILD=true
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

conan install .. --output-folder=. --build=missing --settings=build_type=$BUILD_TYPE
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .

if [ "$RUN_AFTER_BUILD" = true ]; then
  ./enigine_dev
fi

popd
