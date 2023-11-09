#ifndef enigine_hpp
#define enigine_hpp

#include <iostream>
#include <ctime>
#include <bit>

#define IN_PARALLELL_SOLVER

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "external/imgui/imgui.h"
#include "external/stb_image/stb_image.h"

#include "shader/shader.h"
#include "file_manager/file_manager.h"
#include "camera/camera.h"
#include "model/model.h"
#include "sound_engine/sound_engine.h"
#include "physics_world/physics_world.h"
#include "physics_world/debug_drawer/debug_drawer.h"
#include "terrain/terrain.h"
#include "vehicle/vehicle.h"
#include "shadowmap_manager/shadowmap_manager.h"
#include "shadow_manager/shadow_manager.h"
#include "post_process/post_process.h"
#include "pbr_manager/pbr_manager.h"
#include "animation/animator.h"
#include "character_controller/character_controller.h"
#include "ragdoll/ragdoll.h"
#include "utils/bullet_glm.h"
#include "utils/common.h"
#include "ui/root_ui.h"
#include "shader_manager/shader_manager.h"
#include "character/character.h"
#include "character/playable_character.h"
#include "character/np_character.h"
#include "resource_manager/resource_manager.h"
#include "car_controller/car_controller.h"
#include "task_manager/task_manager.h"
#include "render_manager/render_manager.h"
#include "transform/transform.h"
#include "update_manager/update_manager.h"

class Enigine
{
public:
    Enigine();
    ~Enigine();

    PhysicsWorld *physicsWorld;
    // TODO: move into PhysicsWorld
    DebugDrawer *debugDrawer;
    SoundEngine *soundEngine;
    ShaderManager *shaderManager;
    ResourceManager *resourceManager;
    RenderManager *renderManager;
    UpdateManager *updateManager;
    TaskManager *taskManager;
    InputManager *inputManager;

    GLFWwindow *window;
    task_basic_info t_info;
    float deltaTime;

    Camera *mainCamera;

    Shader simpleShader, lineShader, textureArrayShader;

    RootUI *rootUI;
    SystemMonitorUI *systemMonitorUI;
    ShadowmapUI *shadowmapUI;
    RenderUI *renderUI;

    int init();
    void start();

    // TODO: move
    unsigned int vbo, vao, ebo;
    unsigned int q_vbo, q_vao, q_ebo;

private:
    float lastFrame;
};

#endif /* enigine_hpp */
