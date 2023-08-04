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

int main(int argc, char **argv)
{
    CommonUtil::printStartInfo();

    // Setup window
    glfwSetErrorCallback(CommonUtil::glfwErrorCallback);
    if (!glfwInit())
        return 1;

// Decide GL+GLSL versions
#if __APPLE__
    // GL 4.1 + GLSL 410
    const char *glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 4.1 + GLSL 410
    const char *glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *window = glfwCreateWindow(1500, 1000, "enigine", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    bool err = glewInit() != GLEW_OK;

    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // TODO: move to RenderManager
    // Enable depth test, z-buffer
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    // glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    task_basic_info t_info;

    // Init OpenAL
    SoundEngine soundEngine;
    if (!soundEngine.init())
    {
        fprintf(stderr, "Failed to initialize OpenAL!\n");
        return 0;
    }
    soundEngine.setListenerPosition(4.0f, 4.0f, 4.0f);

    // Init Physics
    PhysicsWorld physicsWorld;

    DebugDrawer debugDrawer;
    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe |
                             btIDebugDraw::DBG_DrawConstraints |
                             btIDebugDraw::DBG_DrawConstraintLimits);
    physicsWorld.m_dynamicsWorld->setDebugDrawer(&debugDrawer);

    // Shaders
    ShaderManager shaderManager;
    Shader normalShader, simpleShader, simpleShadow, lineShader, textureShader, textureArrayShader;
    shaderManager.addShader(ShaderDynamic(&normalShader, "../src/assets/shaders/normal-shader.vs", "../src/assets/shaders/normal-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&simpleShader, "../src/assets/shaders/simple-shader.vs", "../src/assets/shaders/simple-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&simpleShadow, "../src/assets/shaders/simple-shadow.vs", "../src/assets/shaders/simple-shadow.fs"));
    shaderManager.addShader(ShaderDynamic(&lineShader, "../src/assets/shaders/line-shader.vs", "../src/assets/shaders/line-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&textureShader, "../src/assets/shaders/simple-texture.vs", "../src/assets/shaders/simple-texture.fs"));
    shaderManager.addShader(ShaderDynamic(&textureArrayShader, "../src/assets/shaders/simple-texture.vs", "../src/assets/shaders/texture-array.fs"));

    // Create geometries
    ResourceManager resourceManager;

    Model &cube = *resourceManager.getModel("../src/assets/models/cube.obj");
    // Model &sphere = *resourceManager.getModel("../src/assets/models/sphere.obj");
    Model &quad = *resourceManager.getModel("../src/assets/models/quad.obj");
    // Model &wheel = *resourceManager.getModel("../src/assets/models/wheel.obj");
    // Model &cylinder = *resourceManager.getModel("../src/assets/models/cylinder.obj");
    // Model &suzanne = *resourceManager.getModel("../src/assets/models/suzanne.obj");
    Model &spherePBR = *resourceManager.getModel("../src/assets/spaceship/sphere.obj");
    Model &shelter = *resourceManager.getModel("../src/assets/gltf/shelter1.glb");
    Model &tower = *resourceManager.getModel("../src/assets/gltf/old-water-tower.glb", false);

    // Shadowmap display quad
    unsigned int q_vbo, q_vao, q_ebo;
    CommonUtil::createQuad(q_vbo, q_vao, q_ebo);

    // Camera
    Camera editorCamera(glm::vec3(10.0f, 3.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Render manager
    RenderManager renderManager(&shaderManager, &editorCamera, &cube, &quad, q_vao);

    // Task manager
    TaskManager taskManager;

    // Characters
    NPCharacter npc1(&renderManager, &taskManager, &resourceManager, &physicsWorld, &editorCamera);
    PCharacter character(&shaderManager, &renderManager, &taskManager, &soundEngine, &resourceManager, &physicsWorld, &editorCamera);

    editorCamera.position = npc1.m_position + glm::vec3(0.f, -1.f, 10.f);

    std::vector<Character *> characters;
    characters.push_back(&character);
    characters.push_back(&npc1);

    character.m_npcList.push_back(&npc1);

    // FollowTask followTask1(&npc1, &character);
    // taskManager.m_tasksFollowCharacter.push_back(followTask1);

    // Time
    float deltaTime = 0.0f;                 // Time between current frame and last frame
    float lastFrame = (float)glfwGetTime(); // Time of last frame

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    // Setup Dear ImGui Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Vehicle
    CarController car(&shaderManager, &renderManager, &physicsWorld, &resourceManager, &editorCamera, character.m_position + glm::vec3(10.f, 5.f, 10.f));
    Vehicle &vehicle = *car.m_vehicle;
    glfwSetWindowUserPointer(window, &car);
    glfwSetKeyCallback(window, car.staticKeyCallback);

    // Debug draw objects
    unsigned int vbo, vao, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Terrain
    // Terrain terrain(&resourceManager, &shaderManager, &physicsWorld, "../src/assets/images/4096x4096.png", 0.0f, 798.0f, 2.0f, true);
    // Terrain terrain(&resourceManager, &shaderManager, &physicsWorld, "../src/assets/images/height-1.png", -2.0f, 517.0f, 6.0f, true);
    // Terrain terrain(&resourceManager, &shaderManager, &physicsWorld, "../src/assets/images/height-2.png", 0.0f, 428.0f, 8.0f, true);
    // Terrain terrain(&resourceManager, &shaderManager, &physicsWorld, "../src/assets/images/height-3.png", 0.0f, 105.0f, 1.0f, true);
    // Terrain terrain(&resourceManager, &shaderManager, &physicsWorld, "../src/assets/images/height-4.png", 0.0f, 508.0f, 1.0f, true);
    Terrain terrain(&resourceManager, &shaderManager, &physicsWorld, "../src/assets/images/test-5.png", -1.0f, 517.0f, 6.0f, true);

    // UI
    RootUI rootUI;
    SystemMonitorUI systemMonitorUI(&t_info);
    CharacterUI characterUI(&character, character.m_controller, character.m_rigidbody);
    RagdollUI ragdollUI(&npc1, &editorCamera);
    AnimationUI animationUI(character.m_animator);
    ShadowmapUI shadowmapUI(renderManager.m_shadowManager, renderManager.m_shadowmapManager);
    SoundUI soundUI(&soundEngine, &character);
    VehicleUI vehicleUI(&car, &vehicle);
    CameraUI cameraUI(&editorCamera);
    TerrainUI terrainUI(&terrain);
    ParticleUI particleUI;
    particleUI.m_particleEngines.push_back(character.m_smokeParticle);
    particleUI.m_particleEngines.push_back(character.m_muzzleFlash);
    particleUI.m_particleEngines.push_back(car.m_exhausParticle);
    ResourceUI resourceUI(&resourceManager);
    RenderUI renderUI(&renderManager);
    TempUI tempUI(renderManager.m_postProcess, &physicsWorld, &debugDrawer, &shaderManager);
    rootUI.m_uiList.push_back(&systemMonitorUI);
    rootUI.m_uiList.push_back(&characterUI);
    rootUI.m_uiList.push_back(&ragdollUI);
    rootUI.m_uiList.push_back(&animationUI);
    rootUI.m_uiList.push_back(&shadowmapUI);
    rootUI.m_uiList.push_back(&soundUI);
    rootUI.m_uiList.push_back(&vehicleUI);
    rootUI.m_uiList.push_back(&cameraUI);
    rootUI.m_uiList.push_back(&terrainUI);
    rootUI.m_uiList.push_back(&particleUI);
    rootUI.m_uiList.push_back(&resourceUI);
    rootUI.m_uiList.push_back(&renderUI);
    rootUI.m_uiList.push_back(&tempUI);

    // Scene
    eTransform transform(glm::vec3(5.f, 5.f, 5.f), glm::quat(1.f, 0.f, 0.f, 0.f), glm::vec3(5.f, 5.f, 5.f));
    renderManager.addSource(RenderSourceBuilder(ShaderType::pbr)
                                .setTransform(transform)
                                .setModel(&spherePBR)
                                .build());

    transform = eTransform();
    renderManager.addTerrainSource(ShaderType::pbr, transform, &terrain);

    transform.setPosition(glm::vec3(103.f, 1.8f, 260.f));
    renderManager.addSource(RenderSourceBuilder(ShaderType::pbr)
                                .setTransform(transform)
                                .setMergedPBRTextures(true)
                                .setModel(&shelter)
                                .build());

    transform.setPosition(glm::vec3(112.f, 18.2f, 233.f));
    transform.setScale(glm::vec3(.1f, .1f, .1f));
    renderManager.addSource(RenderSourceBuilder(ShaderType::pbr)
                                .setTransform(transform)
                                .setMergedPBRTextures(true)
                                .setModel(&tower)
                                .build());

    while (!glfwWindowShouldClose(window))
    {
        // System monitor
        CommonUtil::refreshSystemMonitor(t_info);

        // Calculate deltaTime
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // tempUI.m_deltaTime = deltaTime;

        // Poll events
        glfwPollEvents();

        // Process input
        editorCamera.processInput(window, deltaTime);

        // Update Physics
        physicsWorld.update(deltaTime);

        // Vehicle
        // before character - parent transform
        car.update(window, deltaTime);

        // Update character
        character.update(window, deltaTime);
        // TODO: move
        terrain.m_playerPos = character.m_position;
        npc1.update(window, deltaTime);

        // Update audio listener
        soundEngine.setListenerPosition(editorCamera.position.x, editorCamera.position.y, editorCamera.position.z);
        std::vector<float> listenerOrientation;
        listenerOrientation.push_back(editorCamera.front.x);
        listenerOrientation.push_back(editorCamera.front.y);
        listenerOrientation.push_back(editorCamera.front.z);
        listenerOrientation.push_back(editorCamera.up.x);
        listenerOrientation.push_back(editorCamera.up.y);
        listenerOrientation.push_back(editorCamera.up.z);
        soundEngine.setListenerOrientation(&listenerOrientation);

        // Update tasks - should called at last
        taskManager.update();

        // render manager - start
        renderManager.setupFrame(window);
        // TODO: transform manager?
        renderManager.updateTransforms();
        renderManager.renderDepth();
        renderManager.renderOpaque();

        // Update Debug Drawer
        debugDrawer.getLines().clear();
        physicsWorld.m_dynamicsWorld->debugDrawWorld();

        // Draw physics debug lines
        glm::mat4 mvp = renderManager.m_viewProjection;
        debugDrawer.drawLines(lineShader, mvp, vbo, vao, ebo);

        // culling debug
        renderManager.m_cullingManager->m_debugDrawer->getLines().clear();
        renderManager.m_cullingManager->m_collisionWorld->debugDrawWorld();
        renderManager.m_cullingManager->m_debugDrawer->drawLines(lineShader, mvp, vbo, vao, ebo);

        // Shadowmap debug
        shadowmapUI.drawFrustum(simpleShader, mvp, vbo, vao, ebo);
        shadowmapUI.drawLightAABB(simpleShader, mvp, renderManager.m_inverseDepthViewMatrix, vbo, vao, ebo);

        // character debug
        characterUI.drawArmatureBones(character, simpleShader, cube, mvp);

        // terrain debug
        terrainUI.drawHeightCells(simpleShader, cube, mvp);

        // render manager - end
        renderManager.renderBlend();
        renderManager.renderTransmission();
        renderManager.renderPostProcess();

        // Shadowmap debug - should be called after post process
        shadowmapUI.drawShadowmap(textureArrayShader, renderManager.m_screenW, renderManager.m_screenH, q_vao);

        // Render UI
        rootUI.render();

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // Cleanup glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
