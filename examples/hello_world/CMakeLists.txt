cmake_minimum_required(VERSION 3.17)
project(hello_world)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(hello_world src/main.cpp)

# enigine - internal assets
file(COPY ${CONAN_ENIGINE_ROOT}/assets DESTINATION ${CMAKE_BINARY_DIR}/bin/)
# project assets
# file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/bin/)

target_link_libraries(hello_world ${CONAN_LIBS})
target_compile_features(hello_world PRIVATE cxx_std_20)
