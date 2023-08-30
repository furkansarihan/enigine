cmake_minimum_required(VERSION 3.17)
project(enigine CXX)

# TODO: 
# Define the list of library names
set(config_list
    "glfw3-config"
    "glew-config"
    "glm-config"
    "imgui-config"
    "assimp-config"
    "OpenALconfig"
    "Bulletconfig"
    "SndFileconfig"
    "Boostconfig"
    "Eigen3Config"
)

set(library_name_list
    "glfw3"
    "glew"
    "glm"
    "imgui"
    "assimp"
    "OpenAL"
    "Bullet"
    "SndFile"
    "Boost"
    "Eigen3"
)

set(namespace_list
    "glfw"
    "GLEW"
    "glm"
    "imgui"
    "assimp"
    "OpenAL"
    "Bullet"
    "SndFile"
    "boost"
    "Eigen3"
)

set(library_list
    ""
    "GLEW"
    "glm"
    "imgui"
    "assimp"
    "OpenAL"
    "Bullet"
    "sndfile"
    "boost"
    "Eigen"
)

# Loop through the list and include the corresponding config files
foreach(config ${config_list})
    include(${CMAKE_BINARY_DIR}/generators/${config}.cmake)
endforeach()

# Find each library
foreach(library_name ${library_name_list})
    find_package(${library_name} REQUIRED)
endforeach()

# Add all .cpp files in the src directory and its subdirectories
file(GLOB_RECURSE CPP_SOURCES src/*.cpp)

# Add all .h files in the src directory and its subdirectories
file(GLOB_RECURSE HEADER_FILES src/*.h)

# Create the enigine library target
add_library(enigine ${CPP_SOURCES} ${HEADER_FILES})

# Link each library
foreach(namespace library IN ZIP_LISTS namespace_list library_list)
    if (NOT ${library} STREQUAL "")
        target_link_libraries(enigine ${namespace}::${library})
    else()
        target_link_libraries(enigine ${namespace})
    endif()
endforeach()

# Target enigine library with C++20
target_compile_features(enigine PRIVATE cxx_std_20)

# Install the enigine library
install(TARGETS enigine)

# Install all .h files
foreach(HEADER_FILE ${HEADER_FILES})
    file(RELATIVE_PATH HEADER_REL_PATH ${CMAKE_SOURCE_DIR}/src ${HEADER_FILE})
    get_filename_component(HEADER_DIR ${HEADER_REL_PATH} DIRECTORY)
    install(FILES ${HEADER_FILE} DESTINATION include/${HEADER_DIR})
endforeach()

# Install all internal assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/assets DESTINATION ./)