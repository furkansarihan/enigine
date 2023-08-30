cmake_minimum_required(VERSION 3.0.0)
project(enigine_dev)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Add all .cpp files in the src directory and its subdirectories
file(GLOB_RECURSE CPP_SOURCES ../src/*.cpp)

# Add all .h files in the src directory and its subdirectories
file(GLOB_RECURSE HEADER_FILES ../src/*.h)

# Create the enigine library target
add_executable(enigine_dev src/main.cpp ${CPP_SOURCES} ${HEADER_FILES})

# enigine - internal assets
file(COPY ${CMAKE_SOURCE_DIR}/../src/assets DESTINATION ${CMAKE_BINARY_DIR}/bin/)
# dev - project assets
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/bin/)

target_link_libraries(enigine_dev ${CONAN_LIBS})
target_compile_features(enigine_dev PRIVATE cxx_std_20)
