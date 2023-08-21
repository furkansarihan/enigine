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
                src/vehicle/vehicle.cpp
                src/vehicle/vehicle.h
                src/culling_manager/culling_manager.cpp
                src/culling_manager/culling_manager.h
                src/shadowmap_manager/shadowmap_manager.cpp
                src/shadowmap_manager/shadowmap_manager.h
                src/shadow_manager/shadow_manager.cpp
                src/shadow_manager/shadow_manager.h
                src/post_process/post_process.cpp
                src/post_process/post_process.h
                src/post_process/bloom_manager.h
                src/post_process/bloom_manager.cpp
                src/pbr_manager/pbr_manager.cpp
                src/pbr_manager/pbr_manager.h
                src/utils/assimp_to_glm.h
                src/utils/bullet_glm.h
                src/utils/common.h
                src/animation/bone.h
                src/animation/bone.cpp
                src/animation/animation.h
                src/animation/animation.cpp
                src/animation/animator.h
                src/animation/animator.cpp
                src/character_controller/character_controller.h
                src/character_controller/character_controller.cpp
                src/ragdoll/ragdoll.h
                src/ragdoll/ragdoll.cpp
                src/shader_manager/shader_manager.h
                src/shader_manager/shader_manager.cpp
                src/character/character.h
                src/character/character.cpp
                src/character/playable_character.h
                src/character/playable_character.cpp
                src/character/np_character.h
                src/character/np_character.cpp
                src/character_controller/speed_limiter.h
                src/character_controller/speed_limiter.cpp
                src/particle_engine/particle_engine.h
                src/particle_engine/particle_engine.cpp
                src/resource_manager/resource_manager.h
                src/resource_manager/resource_manager.cpp
                src/car_controller/car_controller.h
                src/car_controller/car_controller.cpp
                src/task_manager/task_manager.h
                src/task_manager/task_manager.cpp
                src/character_task/character_task.h
                src/character_task/follow_task.h
                src/character_task/follow_task.cpp
                src/character_task/follow_path.h
                src/character_task/follow_path.cpp
                src/character_task/enter_car.h
                src/character_task/enter_car.cpp
                src/character_task/exit_car.h
                src/character_task/exit_car.cpp
                src/transform/transform.h
                src/transform/transform.cpp
                src/render_manager/render_manager.h
                src/render_manager/render_manager.cpp
                src/render_manager/g_buffer.h
                src/render_manager/g_buffer.cpp
                src/transform_link/transform_link.h
                src/transform_link/link_rigidbody.h
                src/transform_link/link_rigidbody.cpp
                src/car_controller/link_wheel.h
                src/car_controller/link_wheel.cpp
                src/car_controller/link_door.h
                src/car_controller/link_door.cpp
                # ui
                src/ui/root_ui.h
                src/ui/root_ui.cpp
                src/ui/base_ui.h
                src/ui/animation/animation_ui.h
                src/ui/animation/animation_ui.cpp
                src/ui/camera/camera_ui.h
                src/ui/camera/camera_ui.cpp
                src/ui/character/character_ui.h
                src/ui/character/character_ui.cpp
                src/ui/ragdoll/ragdoll_ui.h
                src/ui/ragdoll/ragdoll_ui.cpp
                src/ui/shadowmap/shadowmap_ui.h
                src/ui/shadowmap/shadowmap_ui.cpp
                src/ui/sound/sound_ui.h
                src/ui/sound/sound_ui.cpp
                src/ui/system_monitor/system_monitor_ui.h
                src/ui/system_monitor/system_monitor_ui.cpp
                src/ui/temp/temp_ui.h
                src/ui/temp/temp_ui.cpp
                src/ui/terrain/terrain_ui.h
                src/ui/terrain/terrain_ui.cpp
                src/ui/vehicle/vehicle_ui.h
                src/ui/vehicle/vehicle_ui.cpp
                src/ui/particle/particle_ui.h
                src/ui/particle/particle_ui.cpp
                src/ui/resource/resource_ui.h
                src/ui/resource/resource_ui.cpp
                src/ui/render/render_ui.h
                src/ui/render/render_ui.cpp
                src/ui/common/vector_ui.h
                src/ui/common/physics_ui.h
                # external
                src/external/jps3d/include/jps_basis/data_type.h
                src/external/jps3d/include/jps_basis/data_utils.h
                src/external/jps3d/include/jps_collision/map_util.h
                src/external/jps3d/include/jps_planner/distance_map_planner/distance_map_planner.h
                src/external/jps3d/include/jps_planner/distance_map_planner/graph_search.h
                src/external/jps3d/include/jps_planner/jps_planner/graph_search.h
                src/external/jps3d/include/jps_planner/jps_planner/jps_planner.h
                src/external/jps3d/src/distance_map_planner/distance_map_planner.cpp
                src/external/jps3d/src/distance_map_planner/graph_search.cpp
                src/external/jps3d/src/jps_planner/graph_search.cpp
                src/external/jps3d/src/jps_planner/jps_planner.cpp
                src/external/imgui/imgui.h
                src/external/imgui/src/imgui_impl_glfw.cpp
                src/external/imgui/src/imgui_impl_glfw.h
                src/external/imgui/src/imgui_impl_opengl3.cpp
                src/external/imgui/src/imgui_impl_opengl3.h
                src/external/imgui/src/imgui_impl_opengl3_loader.h
                src/external/stb_image/stb_image.h )

add_custom_command(TARGET enigine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/assets/models
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/cube.obj ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/sphere.obj ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/wheel.obj ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/cylinder.obj ${PROJECT_BINARY_DIR}/assets/models/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/assets/models/suzanne.obj ${PROJECT_BINARY_DIR}/assets/models/
)

target_link_libraries(enigine ${CONAN_LIBS})
target_compile_features(enigine PRIVATE cxx_std_20)
