@echo off
setlocal

set BUILD_DIR="build"
set CLEAN_BUILD=false
set BUILD_TYPE="Debug"
set RUN_AFTER_BUILD=false

REM Parse command line parameters
for %%i in (%*) do (
  if "%%i"=="--clean" (
    set CLEAN_BUILD=true
  ) else if "%%i"=="--release" (
    set BUILD_TYPE="Release"
  ) else if "%%i"=="--debug" (
    set BUILD_TYPE="Debug"
  ) else if "%%i"=="--run" (
    set RUN_AFTER_BUILD=true
  )
)

REM Clean build directory if requested
if %CLEAN_BUILD%==true (
  rmdir /s /q %BUILD_DIR%
  mkdir %BUILD_DIR%
)

REM Create the build directory if it does not exist
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

pushd %BUILD_DIR%

conan install .. --output-folder=. --build=missing --settings=build_type=%BUILD_TYPE%
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build . --config %BUILD_TYPE%

if %RUN_AFTER_BUILD%==true (
  enigine_dev.exe
)

popd

endlocal
