#!/bin/bash

set -e
set -x

BASE_BUILD_DIR="build"
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

# Set build directory based on build type
BUILD_DIR="${BASE_BUILD_DIR}/${BUILD_TYPE}"

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
  rm -rf $BUILD_DIR
fi

# Create the build directory if it does not exist
mkdir -p $BUILD_DIR

pushd $BUILD_DIR

conan install ../.. --output-folder=. --build=missing --settings=build_type=$BUILD_TYPE
cmake ../.. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build . --parallel 8

if [ "$RUN_AFTER_BUILD" = true ]; then
  ./enigine_dev
fi

popd
