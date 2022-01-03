cmake_minimum_required(VERSION 3.0.0)
project(enigine)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# CONFIG option is important so that CMake doesnt search for modules into the default modules directory
find_package(imgui CONFIG)
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
                src/mesh/mesh.cpp
                src/mesh/mesh.h
                src/model/model.cpp
                src/model/model.h
                # external
                src/external/imgui/imgui.h
                src/external/imgui/src/imgui_impl_glfw.cpp
                src/external/imgui/src/imgui_impl_glfw.h
                src/external/imgui/src/imgui_impl_opengl3.cpp
                src/external/imgui/src/imgui_impl_opengl3.h
                src/external/imgui/src/imgui_impl_opengl3_loader.h
                src/external/stb_image/stb_image.h
                # assets
                src/assets/shaders/simple-shader.vs 
                src/assets/shaders/simple-shader.fs
                src/assets/shaders/light-shader.vs 
                src/assets/shaders/light-shader.fs
                src/assets/models/cube.obj )

add_custom_command(TARGET enigine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/models
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/simple-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/simple-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/light-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/light-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/cube.obj ${PROJECT_BINARY_DIR}/assets/models/
)

target_link_libraries(enigine ${CONAN_LIBS})
