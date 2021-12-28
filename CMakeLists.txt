cmake_minimum_required(VERSION 3.0.0)
project(enigine)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(enigine src/enigine.cpp)
target_link_libraries(enigine ${CONAN_LIBS})
