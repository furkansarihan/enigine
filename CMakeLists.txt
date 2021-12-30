cmake_minimum_required(VERSION 3.0.0)
project(enigine)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# CONFIG option is important so that CMake doesnt search for modules into the default modules directory
find_package(glfw3 CONFIG)
find_package(glew CONFIG)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable( enigine
                # main
                src/enigine.cpp
                # components
                src/shader/shader.cpp
                src/shader/shader.h
                src/file_manager/file_manager.cpp
                src/file_manager/file_manager.h
                src/camera/camera.cpp
                src/camera/camera.h
                # assets
                src/assets/simple-shader.vs 
                src/assets/simple-shader.fs )

add_custom_command(TARGET enigine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/simple-shader.vs ${PROJECT_BINARY_DIR}/assets/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/simple-shader.fs ${PROJECT_BINARY_DIR}/assets/
)

target_link_libraries(enigine ${CONAN_LIBS})
