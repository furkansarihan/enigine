cmake_minimum_required(VERSION 3.0.0)
project(enigine)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

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
                src/sound_engine/sound_engine.cpp
                src/sound_engine/sound_engine.h
                src/physics_world/physics_world.cpp
                src/physics_world/physics_world.h
                src/physics_world/debug_drawer/debug_drawer.cpp
                src/physics_world/debug_drawer/debug_drawer.h
                src/terrain/terrain.cpp
                src/terrain/terrain.h
                # external
                src/external/imgui/imgui.h
                src/external/imgui/src/imgui_impl_glfw.cpp
                src/external/imgui/src/imgui_impl_glfw.h
                src/external/imgui/src/imgui_impl_opengl3.cpp
                src/external/imgui/src/imgui_impl_opengl3.h
                src/external/imgui/src/imgui_impl_opengl3_loader.h
                src/external/stb_image/stb_image.h
                src/external/dr_wav/dr_wav.h )

add_custom_command(TARGET enigine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/models
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/sounds
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/images
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/simple-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/simple-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/light-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/light-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/normal-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/normal-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/line-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/line-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/terrain-shader.vs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/shaders/terrain-shader.fs ${PROJECT_BINARY_DIR}/assets/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/cube.obj ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/sphere.obj ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/sphere.mtl ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/sphere_diffuse.jpg ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/sphere_normal.png ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/sounds/rain-loop-1648m.wav ${PROJECT_BINARY_DIR}/assets/sounds/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/4096x4096.png ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/4096x4096-normal.png ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/4096x4096-color.png ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/water-1.jpg ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/sand-1.jpg ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/stone-1.jpg ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/grass-1.jpg ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/rock-1.jpg ${PROJECT_BINARY_DIR}/assets/images/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/images/snow-1.jpg ${PROJECT_BINARY_DIR}/assets/images/
)

target_link_libraries(enigine ${CONAN_LIBS})
target_compile_features(enigine PRIVATE cxx_std_20)
