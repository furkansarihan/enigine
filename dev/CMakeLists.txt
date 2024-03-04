cmake_minimum_required(VERSION 3.15)
project(enigine_dev)

# Define the path to the enigine directory
set(ENIGINE_DIR "${CMAKE_SOURCE_DIR}/..")

# enigine - find libraries
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Bullet REQUIRED)
find_package(drwav REQUIRED)
# dev - find libraries
# find_package(... REQUIRED)

# Add all .cpp files in the src directory and its subdirectories
file(GLOB_RECURSE CPP_SOURCES ${ENIGINE_DIR}/src/*.cpp src/*.cpp)

# Add all .h files in the src directory and its subdirectories
file(GLOB_RECURSE HEADER_FILES ${ENIGINE_DIR}/src/*.h src/*.h)

# Create the enigine library target
add_executable(${PROJECT_NAME} src/main.cpp ${CPP_SOURCES} ${HEADER_FILES})

# Include enigine headers
target_include_directories(${PROJECT_NAME} PRIVATE ${ENIGINE_DIR}/src)

# enigine - internal assets
file(COPY ${ENIGINE_DIR}/src/assets DESTINATION ${CMAKE_BINARY_DIR}/)
# dev - project assets
# file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/)

# enigine - link libraries
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} imgui::imgui)
target_link_libraries(${PROJECT_NAME} assimp::assimp)
target_link_libraries(${PROJECT_NAME} OpenAL::OpenAL)
target_link_libraries(${PROJECT_NAME} Bullet::Bullet)
target_link_libraries(${PROJECT_NAME} drwav::drwav)
# dev - link libraries
# target_link_libraries(${PROJECT_NAME} ...)
