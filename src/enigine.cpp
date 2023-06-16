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

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    // Enable depth test, z-buffer
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    // glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);
    // Trasparency
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    physicsWorld.m_dynamicsWorld->setDebugDrawer(&debugDrawer);

    // Shaders
    ShaderManager shaderManager;
    Shader normalShader, simpleShader, depthShader, simpleShadow, terrainShader, terrainShadow, lineShader, textureShader, textureArrayShader, postProcessShader, hdrToCubemapShader, cubemapShader, irradianceShader, pbrShader, prefilterShader, brdfShader, grassShader, stoneShader, animShader, texture2Shader, smokeShader, muzzleFlashShader, terrainPBRShader, animDepthShader, exhaustShader;
    shaderManager.addShader(ShaderDynamic(&normalShader, "../src/assets/shaders/normal-shader.vs", "../src/assets/shaders/normal-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&simpleShader, "../src/assets/shaders/simple-shader.vs", "../src/assets/shaders/simple-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&depthShader, "../src/assets/shaders/simple-shader.vs", "../src/assets/shaders/depth-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&simpleShadow, "../src/assets/shaders/simple-shadow.vs", "../src/assets/shaders/simple-shadow.fs"));
    shaderManager.addShader(ShaderDynamic(&terrainShader, "../src/assets/shaders/terrain-shader.vs", "../src/assets/shaders/terrain-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&terrainShadow, "../src/assets/shaders/terrain-shadow.vs", "../src/assets/shaders/depth-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&lineShader, "../src/assets/shaders/line-shader.vs", "../src/assets/shaders/line-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&textureShader, "../src/assets/shaders/simple-texture.vs", "../src/assets/shaders/simple-texture.fs"));
    shaderManager.addShader(ShaderDynamic(&textureArrayShader, "../src/assets/shaders/simple-texture.vs", "../src/assets/shaders/texture-array.fs"));
    shaderManager.addShader(ShaderDynamic(&postProcessShader, "../src/assets/shaders/post-process.vs", "../src/assets/shaders/post-process.fs"));
    shaderManager.addShader(ShaderDynamic(&hdrToCubemapShader, "../src/assets/shaders/hdr-to-cubemap.vs", "../src/assets/shaders/hdr-to-cubemap.fs"));
    shaderManager.addShader(ShaderDynamic(&cubemapShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/cubemap.fs"));
    shaderManager.addShader(ShaderDynamic(&irradianceShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/irradiance.fs"));
    shaderManager.addShader(ShaderDynamic(&pbrShader, "../src/assets/shaders/pbr.vs", "../src/assets/shaders/pbr.fs"));
    shaderManager.addShader(ShaderDynamic(&prefilterShader, "../src/assets/shaders/cubemap.vs", "../src/assets/shaders/prefilter.fs"));
    shaderManager.addShader(ShaderDynamic(&brdfShader, "../src/assets/shaders/post-process.vs", "../src/assets/shaders/brdf.fs"));
    shaderManager.addShader(ShaderDynamic(&grassShader, "../src/assets/shaders/grass.vs", "../src/assets/shaders/grass.fs"));
    shaderManager.addShader(ShaderDynamic(&stoneShader, "../src/assets/shaders/stone.vs", "../src/assets/shaders/stone.fs"));
    shaderManager.addShader(ShaderDynamic(&animShader, "../src/assets/shaders/anim.vs", "../src/assets/shaders/anim.fs"));
    shaderManager.addShader(ShaderDynamic(&texture2Shader, "../src/assets/shaders/texture-2.vs", "../src/assets/shaders/anim.fs"));
    shaderManager.addShader(ShaderDynamic(&smokeShader, "../src/assets/shaders/smoke.vs", "../src/assets/shaders/smoke.fs"));
    shaderManager.addShader(ShaderDynamic(&muzzleFlashShader, "../src/assets/shaders/muzzle-flash.vs", "../src/assets/shaders/muzzle-flash.fs"));
    shaderManager.addShader(ShaderDynamic(&terrainPBRShader, "../src/assets/shaders/terrain-shader.vs", "../src/assets/shaders/terrain-pbr.fs"));
    shaderManager.addShader(ShaderDynamic(&animDepthShader, "../src/assets/shaders/anim.vs", "../src/assets/shaders/depth-shader.fs"));
    shaderManager.addShader(ShaderDynamic(&exhaustShader, "../src/assets/shaders/smoke.vs", "../src/assets/shaders/exhaust.fs"));

    // Create geometries
    ResourceManager resourceManager;

    Model &cube = *resourceManager.getModel("../src/assets/models/cube.obj");
    Model &sphere = *resourceManager.getModel("../src/assets/models/sphere.obj");
    Model &quad = *resourceManager.getModel("../src/assets/models/quad.obj");
    Model &wheel = *resourceManager.getModel("../src/assets/models/wheel.obj");
    Model &cylinder = *resourceManager.getModel("../src/assets/models/cylinder.obj");
    Model &suzanne = *resourceManager.getModel("../src/assets/models/suzanne.obj");
    // Model &spherePBR = *resourceManager.getModel("../src/assets/spaceship/sphere.obj");
    Model &grass = *resourceManager.getModel("../src/assets/terrain/grass.obj");
    Model &stone = *resourceManager.getModel("../src/assets/terrain/stone.obj");
    Model &pistol = *resourceManager.getModel("../src/assets/gltf/colt3.glb");

    // car
    Model &carBody = *resourceManager.getModel("../src/assets/car/body.gltf");
    Model &carHood = *resourceManager.getModel("../src/assets/car/hood.gltf");
    Model &carTrunk = *resourceManager.getModel("../src/assets/car/trunk.gltf");
    Model &carWheelFL = *resourceManager.getModel("../src/assets/car/wheel-fl.gltf");
    Model &carWheelFR = *resourceManager.getModel("../src/assets/car/wheel-fr.gltf");
    Model &carWheelRL = *resourceManager.getModel("../src/assets/car/wheel-rl.gltf");
    Model &carWheelRR = *resourceManager.getModel("../src/assets/car/wheel-rr.gltf");
    Model *wheels[4] = {&carWheelFL, &carWheelFR, &carWheelRL, &carWheelRR};
    Model &carDoorFL = *resourceManager.getModel("../src/assets/car/door-fl.gltf");
    Model &carDoorFR = *resourceManager.getModel("../src/assets/car/door-fr.gltf");
    Model &carDoorRL = *resourceManager.getModel("../src/assets/car/door-rl.gltf");
    Model &carDoorRR = *resourceManager.getModel("../src/assets/car/door-rr.gltf");
    Model *doors[4] = {&carDoorFL, &carDoorFR, &carDoorRL, &carDoorRR};

    // Camera
    Camera editorCamera(glm::vec3(10.0f, 3.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Camera debugCamera(glm::vec3(10.0f, 3.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Characters
    NPCharacter npc1(&resourceManager, &shaderManager, &physicsWorld, &editorCamera);
    PCharacter character(&soundEngine, &resourceManager, &shaderManager, &physicsWorld, &editorCamera);

    std::vector<Character *> characters;
    characters.push_back(&character);
    characters.push_back(&npc1);

    npc1.m_avoidAimList.push_back(&character);
    character.m_npcList.push_back(&npc1);

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
    CarController car(&physicsWorld, &resourceManager, &editorCamera, character.m_position + glm::vec3(40.f, 5.f, 40.f));
    Vehicle &vehicle = *car.m_vehicle;
    glfwSetWindowUserPointer(window, &car);
    glfwSetKeyCallback(window, car.staticKeyCallback);

    // Shadowmap
    std::vector<unsigned int> shaderIds;
    shaderIds.push_back(simpleShadow.id);
    shaderIds.push_back(terrainShadow.id);

    ShadowManager shadowManager(&editorCamera, shaderIds);
    ShadowmapManager shadowmapManager(shadowManager.m_splitCount, 1024);

    // Shadowmap display quad
    unsigned int q_vbo, q_vao, q_ebo;
    CommonUtil::createQuad(q_vbo, q_vao, q_ebo);

    // Debug draw objects
    unsigned int vbo, vao, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Post process
    PostProcess postProcess((float)screenWidth, (float)screenHeight);

    // PBR
    PbrManager pbrManager;
    pbrManager.setupCubemap(cube, hdrToCubemapShader);
    pbrManager.setupIrradianceMap(cube, irradianceShader);
    pbrManager.setupPrefilterMap(cube, prefilterShader);
    pbrManager.setupBrdfLUTTexture(q_vao, brdfShader);

    float albedo[3] = {0.5f, 0.0f, 0.0f};
    float ao = 1.0;
    glm::vec3 lightPositions[] = {glm::vec3(0.0f, 0.0f, 10.0f)};
    glm::vec3 lightColors[] = {glm::vec3(350.0f, 410.0f, 458.0f)};

    // Terrain
    // Terrain terrain(&pbrManager, &physicsWorld, "../src/assets/images/4096x4096.png", 0.0f, 798.0f, 2.0f, true);
    // Terrain terrain(&pbrManager, &physicsWorld, "../src/assets/images/height-1.png", -2.0f, 517.0f, 6.0f, true);
    // Terrain terrain(&pbrManager, &physicsWorld, "../src/assets/images/height-2.png", 0.0f, 428.0f, 8.0f, true);
    // Terrain terrain(&pbrManager, &physicsWorld, "../src/assets/images/height-3.png", 0.0f, 105.0f, 1.0f, true);
    // Terrain terrain(&pbrManager, &physicsWorld, "../src/assets/images/height-4.png", 0.0f, 508.0f, 1.0f, true);
    Terrain terrain(&pbrManager, &physicsWorld, "../src/assets/images/test-5.png", -1.0f, 517.0f, 6.0f, true);

    // UI
    RootUI rootUI;
    SystemMonitorUI systemMonitorUI(&t_info);
    CharacterUI characterUI(&character, character.m_controller, character.m_rigidbody);
    RagdollUI ragdollUI(&npc1, &editorCamera);
    AnimationUI animationUI(character.m_animator);
    ShadowmapUI shadowmapUI(&shadowManager, &shadowmapManager);
    SoundUI soundUI(&soundEngine, &character);
    VehicleUI vehicleUI(&car, &vehicle);
    CameraUI cameraUI(&editorCamera);
    TerrainUI terrainUI(&terrain);
    ParticleUI particleUI;
    particleUI.m_particleEngines.push_back(character.m_smokeParticle);
    particleUI.m_particleEngines.push_back(character.m_muzzleFlash);
    particleUI.m_particleEngines.push_back(car.m_exhausParticle);
    TempUI tempUI(&postProcess, &physicsWorld, &debugDrawer, &shaderManager);
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
    rootUI.m_uiList.push_back(&tempUI);

    while (!glfwWindowShouldClose(window))
    {
        // System monitor
        CommonUtil::refreshSystemMonitor(t_info);

        // Calculate deltaTime
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        tempUI.m_deltaTime = deltaTime;

        // Poll events
        glfwPollEvents();

        // Process input
        editorCamera.processInput(window, deltaTime);

        // Update character
        character.update(window, deltaTime);
        npc1.update(window, deltaTime);

        // Update Physics
        physicsWorld.update(deltaTime);

        // Update Debug Drawer
        debugDrawer.getLines().clear();
        physicsWorld.m_dynamicsWorld->debugDrawWorld();

        // Vehicle
        car.update(window, deltaTime);

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

        // Clear window
        glClearColor(0.46f, 0.71f, 0.98f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update projection
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        glm::mat4 projection = editorCamera.getProjectionMatrix((float)screenWidth, (float)screenHeight);

        // Shadowmap
        shadowManager.setup((float)screenWidth, (float)screenHeight);
        glm::mat4 depthViewMatrix = shadowManager.getDepthViewMatrix();
        glm::mat4 inverseDepthViewMatrix = glm::inverse(depthViewMatrix);

        shadowmapManager.bindFramebuffer();
        for (int i = 0; i < shadowManager.m_splitCount; i++)
        {
            shadowmapManager.bindTextureArray(i);
            glm::mat4 depthP = shadowManager.m_depthPMatrices[i];
            glm::mat4 depthVP = depthP * depthViewMatrix;

            // Draw terrain
            glm::vec3 nearPlaneEdges[4];
            for (int j = 0; j < 4; j++)
            {
                glm::vec4 worldPoint = inverseDepthViewMatrix * glm::vec4(shadowManager.m_frustums[i].lightAABB[j], 1.0f);
                glm::vec3 worldPoint3 = glm::vec3(worldPoint) / worldPoint.w;
                nearPlaneEdges[j] = worldPoint3;
            }
            glm::vec3 nearPlaneCenter = (nearPlaneEdges[0] + nearPlaneEdges[1] + nearPlaneEdges[2] + nearPlaneEdges[3]) / 4.0f;

            // TODO: terrain only cascade - covers all area - last one
            terrain.drawDepth(terrainShadow, depthViewMatrix, depthP, nearPlaneCenter);

            // Draw objects
            glm::vec4 objectColor(0.6f, 0.6f, 0.6f, 1.0f);

            glm::vec3 position = glm::vec3(0, 2, 0);
            glm::vec3 scale = glm::vec3(4, 4, 4);
            glm::mat4 objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.use();
            depthShader.setMat4("MVP", depthVP * objectModel);
            suzanne.draw(depthShader);

            // wall
            position = glm::vec3(0, 1.25, -9);
            scale = glm::vec3(10.0f, 4.0f, 1.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.setMat4("MVP", depthVP * objectModel);
            cube.draw(depthShader);

            // ground
            position = glm::vec3(0, 0, 0.75);
            scale = glm::vec3(20.0f, 2.0f, 100.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.setMat4("MVP", depthVP * objectModel);
            cube.draw(depthShader);

            // ball
            for (int i = 2; i < 30; i++)
            {
                position = glm::vec3(0, 2, 3 * i);
                scale = glm::vec3(2.0f, 2.0f, 2.0f);
                objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
                depthShader.setMat4("MVP", depthVP * objectModel);
                sphere.draw(depthShader);
            }

            // ball
            for (int i = 2; i < 30; i++)
            {
                position = glm::vec3(-20, 2, 3 * i);
                scale = glm::vec3(2.0f, 2.0f, 2.0f);
                objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
                depthShader.setMat4("MVP", depthVP * objectModel);
                sphere.draw(depthShader);
            }

            // vehicle
            glm::mat4 model = vehicle.m_chassisModel;
            model = glm::translate(model, car.m_bodyOffset);
            model = model * car.m_carModel;
            depthShader.setMat4("MVP", depthVP * model);
            carBody.draw(depthShader);

            for (int i = 0; i < 4; i++)
            {
                glm::mat4 model;
                vehicle.m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix((btScalar *)&model);
                model = glm::translate(model, car.m_wheelOffset);
                model = glm::rotate(model, car.m_rotation.x, glm::vec3(1, 0, 0));
                model = glm::rotate(model, car.m_rotation.y, glm::vec3(0, 1, 0));
                model = glm::rotate(model, car.m_rotation.z, glm::vec3(0, 0, 1));
                model = glm::scale(model, glm::vec3(car.m_wheelScale));
                depthShader.setMat4("MVP", depthVP * model);
                wheels[i]->draw(depthShader);
            }

            // hood
            model = vehicle.m_chassisModel;
            model = glm::translate(model, car.m_hoodOffset);
            model = model * car.m_carModel;
            depthShader.setMat4("MVP", depthVP * model);
            carHood.draw(depthShader);

            // trunk
            model = vehicle.m_chassisModel;
            model = glm::translate(model, car.m_trunkOffset);
            model = model * car.m_carModel;
            depthShader.setMat4("MVP", depthVP * model);
            carTrunk.draw(depthShader);

            // doors
            for (int i = 0; i < 4; i++)
            {
                glm::mat4 model = vehicle.m_chassisModel;
                model = glm::translate(model, car.m_doorOffsets[i]);
                model = model * car.m_carModel;
                depthShader.setMat4("MVP", depthVP * model);
                doors[i]->draw(depthShader);
            }

            // characters
            animDepthShader.use();
            animDepthShader.setMat4("projection", depthP);
            animDepthShader.setMat4("view", depthViewMatrix);

            for (int i = 0; i < characters.size(); i++)
            {
                Character *character = characters[i];

                auto transforms = character->m_animator->m_FinalBoneMatrices;
                for (int i = 0; i < transforms.size(); ++i)
                {
                    animDepthShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
                }

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, character->m_position);
                model = glm::rotate(model, character->m_rotation.x, glm::vec3(1, 0, 0));
                model = glm::rotate(model, character->m_rotation.y * (1.0f - character->getRagdolPose().blendFactor), glm::vec3(0, 1, 0));
                model = glm::rotate(model, character->m_rotation.z, glm::vec3(0, 0, 1));
                model = glm::scale(model, glm::vec3(character->m_scale));
                animDepthShader.setMat4("model", model);
                character->m_model->draw(animDepthShader);
            }
        }

        // Render to post process texture
        postProcess.updateFramebuffer((float)screenWidth, (float)screenHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, postProcess.m_framebufferObject);
        glViewport(0, 0, screenWidth, screenHeight);
        glClearColor(0.46f, 0.71f, 0.98f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO: pbr
        // animation
        for (int i = 0; i < characters.size(); i++)
        {
            Character *character = characters[i];
            animShader.use();
            animShader.setMat4("projection", projection);
            animShader.setMat4("view", editorCamera.getViewMatrix());
            animShader.setVec3("lightDir", shadowManager.m_lightPos);
            animShader.setVec3("lightColor", glm::vec3(tempUI.m_lightColor[0], tempUI.m_lightColor[1], tempUI.m_lightColor[2]));
            animShader.setFloat("lightPower", tempUI.m_lightPower);

            auto transforms = character->m_animator->m_FinalBoneMatrices;
            for (int i = 0; i < transforms.size(); ++i)
            {
                animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            }

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, character->m_position);
            model = glm::rotate(model, character->m_rotation.x, glm::vec3(1, 0, 0));
            model = glm::rotate(model, character->m_rotation.y * (1.0f - character->getRagdolPose().blendFactor), glm::vec3(0, 1, 0));
            model = glm::rotate(model, character->m_rotation.z, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(character->m_scale));
            animShader.setMat4("model", model);
            character->m_model->draw(animShader);
        }

        // Render scene
        glm::vec4 frustumDistances = shadowManager.getFrustumDistances();

        glm::mat4 cullProjection = projection;
        glm::mat4 cullView = editorCamera.getViewMatrix();
        glm::vec3 cullViewPos = editorCamera.position;

        // TODO: move where?
        if (terrainUI.m_debugCulling)
        {
            cullProjection = debugCamera.getProjectionMatrix(screenWidth, screenHeight);
            cullView = debugCamera.getViewMatrix();
            cullViewPos = debugCamera.position;
        }
        else
        {
            debugCamera.position = editorCamera.position;
            debugCamera.front = editorCamera.front;
            debugCamera.right = editorCamera.right;
            debugCamera.up = editorCamera.up;
        }

        terrain.drawColor(terrainPBRShader, shadowManager.m_lightPos, tempUI.m_sunColor * tempUI.m_sunIntensity,
                          tempUI.m_lightPower,
                          editorCamera.getViewMatrix(), projection, editorCamera.position,
                          cullView, cullProjection, cullViewPos,
                          shadowmapManager.m_textureArray,
                          editorCamera.position, editorCamera.front,
                          frustumDistances,
                          editorCamera.projectionMode == ProjectionMode::Ortho);

        terrain.drawInstance(grassShader, &grass, terrain.m_grassTileSize, terrain.m_grassDensity, projection, editorCamera.getViewMatrix(), editorCamera.position);
        terrain.drawInstance(stoneShader, &stone, terrain.m_stoneTileSize, terrain.m_stoneDensity, projection, editorCamera.getViewMatrix(), editorCamera.position);

        // Draw objects
        {
            glm::vec3 objectColor(0.6f, 0.6f, 0.6f);

            simpleShadow.use();
            simpleShadow.setFloat("biasMult", shadowManager.m_bias);
            simpleShadow.setVec3("AmbientColor", glm::vec3(tempUI.m_ambientColor[0], tempUI.m_ambientColor[1], tempUI.m_ambientColor[2]));
            simpleShadow.setVec3("DiffuseColor", objectColor);
            simpleShadow.setVec3("SpecularColor", glm::vec3(tempUI.m_specularColor[0], tempUI.m_specularColor[1], tempUI.m_specularColor[2]));
            simpleShadow.setVec3("LightColor", glm::vec3(tempUI.m_lightColor[0], tempUI.m_lightColor[1], tempUI.m_lightColor[2]));
            simpleShadow.setFloat("LightPower", tempUI.m_lightPower);
            simpleShadow.setVec3("CamPos", shadowManager.m_camera->position);
            simpleShadow.setVec3("CamView", shadowManager.m_camera->front);
            simpleShadow.setVec4("FrustumDistances", frustumDistances);
            simpleShadow.setBool("ShowCascade", terrain.showCascade);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(simpleShadow.id, "ShadowMap"), 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, shadowmapManager.m_textureArray);

            // suzanne
            glm::vec3 position = glm::vec3(0, 2, 0);
            glm::vec3 scale = glm::vec3(4, 4, 4);
            glm::mat4 objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", shadowManager.m_lightPos);
            suzanne.draw(simpleShadow);

            // wall
            position = glm::vec3(0, 1.25, -9);
            scale = glm::vec3(10.0f, 4.0f, 1.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", shadowManager.m_lightPos);
            cube.draw(simpleShadow);

            // ground
            position = glm::vec3(0, 0, 0.75);
            scale = glm::vec3(20.0f, 2.0f, 100.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", shadowManager.m_lightPos);
            cube.draw(simpleShadow);

            // ball
            for (int i = 2; i < 30; i++)
            {
                position = glm::vec3(0, 2, 3 * i);
                scale = glm::vec3(2.0f, 2.0f, 2.0f);
                objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
                simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
                simpleShadow.setMat4("V", editorCamera.getViewMatrix());
                simpleShadow.setMat4("M", objectModel);
                simpleShadow.setVec3("LightInvDirection_worldspace", shadowManager.m_lightPos);
                sphere.draw(simpleShadow);
            }

            // ball
            for (int i = 2; i < 30; i++)
            {
                position = glm::vec3(-20, 2, 3 * i);
                scale = glm::vec3(2.0f, 2.0f, 2.0f);
                objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
                simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
                simpleShadow.setMat4("V", editorCamera.getViewMatrix());
                simpleShadow.setMat4("M", objectModel);
                simpleShadow.setVec3("LightInvDirection_worldspace", shadowManager.m_lightPos);
                sphere.draw(simpleShadow);
            }
        }

        // animate light source
        lightPositions[0].x = tempUI.m_radius * glm::sin(currentFrame * tempUI.m_speed);
        lightPositions[0].y = tempUI.m_radius * glm::sin(currentFrame * (tempUI.m_speed / 6)) + 20;
        lightPositions[0].z = tempUI.m_radius * glm::cos(currentFrame * tempUI.m_speed) + 6;

        // draw pbr
        // TODO: toggle
        {
            pbrShader.use();
            glm::mat4 view = editorCamera.getViewMatrix();
            pbrShader.setMat4("view", view);
            pbrShader.setMat4("projection", projection);
            pbrShader.setVec3("camPos", editorCamera.position);
            pbrShader.setVec3("albedo", albedo[0], albedo[1], albedo[2]);
            pbrShader.setFloat("ao", ao);
            pbrShader.setVec3("lightDirection", shadowManager.m_lightPos);
            pbrShader.setVec3("lightColor", tempUI.m_sunColor * tempUI.m_sunIntensity);

            for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
            {
                pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
                pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
            }

            glActiveTexture(GL_TEXTURE0 + 7);
            glUniform1i(glGetUniformLocation(pbrShader.id, "irradianceMap"), 7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, pbrManager.irradianceMap);

            glActiveTexture(GL_TEXTURE0 + 8);
            glUniform1i(glGetUniformLocation(pbrShader.id, "prefilterMap"), 8);
            glBindTexture(GL_TEXTURE_CUBE_MAP, pbrManager.prefilterMap);

            glActiveTexture(GL_TEXTURE0 + 9);
            glUniform1i(glGetUniformLocation(pbrShader.id, "brdfLUT"), 9);
            glBindTexture(GL_TEXTURE_2D, pbrManager.brdfLUTTexture);

            float nrRows = 16;
            float nrColumns = 4;
            float spacing = 5;

            glm::mat4 model = glm::mat4(1.0f);
            for (int row = 10; row < nrRows; ++row)
            {
                pbrShader.setFloat("metallic", (float)row / (float)nrRows);
                for (int col = 0; col < nrColumns; ++col)
                {
                    // we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                    // on direct lighting.
                    pbrShader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

                    model = glm::mat4(1.0f);
                    glm::vec3 position(
                        (col - (nrColumns / 2)) * spacing + spacing / 2,
                        (row - (nrRows / 2)) * spacing,
                        6);
                    model = glm::translate(model, position);
                    model = glm::scale(model, glm::vec3(2));
                    pbrShader.setMat4("model", model);
                    pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                    // spherePBR.draw(pbrShader);
                }
            }

            // draw pistol
            pbrShader.setMat4("model", character.m_pistolModel);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(character.m_pistolModel))));
            pbrShader.setBool("mergedPBRTextures", true);
            pistol.draw(pbrShader);

            // draw vehicle
            // body
            model = vehicle.m_chassisModel;
            model = glm::translate(model, car.m_bodyOffset);
            model = model * car.m_carModel;
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            carBody.draw(pbrShader);

            // hood
            model = vehicle.m_chassisModel;
            model = glm::translate(model, car.m_hoodOffset);
            model = model * car.m_carModel;
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            carHood.draw(pbrShader);

            // trunk
            model = vehicle.m_chassisModel;
            model = glm::translate(model, car.m_trunkOffset);
            model = model * car.m_carModel;
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            carTrunk.draw(pbrShader);

            // doors
            for (int i = 0; i < 4; i++)
            {
                glm::mat4 model = vehicle.m_chassisModel;
                model = glm::translate(model, car.m_doorOffsets[i]);
                model = model * car.m_carModel;
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                doors[i]->draw(pbrShader);
            }

            // wheels
            for (int i = 0; i < 4; i++)
            {
                glm::mat4 model;
                vehicle.m_vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix((btScalar *)&model);
                model = glm::translate(model, car.m_wheelOffset);
                model = glm::rotate(model, car.m_rotation.x, glm::vec3(1, 0, 0));
                model = glm::rotate(model, car.m_rotation.y, glm::vec3(0, 1, 0));
                model = glm::rotate(model, car.m_rotation.z, glm::vec3(0, 0, 1));
                model = glm::scale(model, glm::vec3(car.m_wheelScale));
                pbrShader.setMat4("model", model);
                pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                wheels[i]->draw(pbrShader);
            }

            // light sources
            for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, lightPositions[i]);
                model = glm::scale(model, glm::vec3(0.2f));

                simpleShader.use();
                simpleShader.setMat4("MVP", projection * editorCamera.getViewMatrix() * model);
                simpleShader.setVec4("DiffuseColor", glm::vec4(1.0, 1.0, 1.0, 1.0f));
                sphere.draw(simpleShader);
            }
        }

        // Draw light source
        glm::mat4 mvp = projection * editorCamera.getViewMatrix() * glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)), shadowManager.m_lightPos);
        simpleShader.use();
        simpleShader.setMat4("MVP", mvp);
        simpleShader.setVec4("DiffuseColor", glm::vec4(tempUI.m_lightColor[0], tempUI.m_lightColor[1], tempUI.m_lightColor[2], 1.0f));
        sphere.draw(simpleShader);

        // Draw physics debug lines
        mvp = projection * editorCamera.getViewMatrix();
        debugDrawer.drawLines(lineShader, mvp, vbo, vao, ebo);

        // Shadowmap debug
        shadowmapUI.drawFrustum(simpleShader, mvp, vbo, vao, ebo);
        shadowmapUI.drawLightAABB(simpleShader, mvp, inverseDepthViewMatrix, vbo, vao, ebo);

        // character debug
        characterUI.drawArmatureBones(character, simpleShader, cube, projection * editorCamera.getViewMatrix());

        // terrain debug
        terrainUI.drawHeightCells(simpleShader, cube, projection * editorCamera.getViewMatrix());

        // Draw skybox
        glDepthMask(GL_FALSE);
        cubemapShader.use();
        cubemapShader.setMat4("projection", projection);
        cubemapShader.setMat4("view", editorCamera.getViewMatrix());

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(cubemapShader.id, "environmentMap"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pbrManager.m_skyboxTexture);

        cube.draw(cubemapShader);
        glDepthMask(GL_TRUE);

        // Draw particles
        // TODO: ParticleManager
        character.m_smokeParticle->drawParticles(smokeShader, quad, projection * editorCamera.getViewMatrix());
        character.m_muzzleFlash->drawParticles(muzzleFlashShader, quad, projection * editorCamera.getViewMatrix());
        car.m_exhausParticle->drawParticles(exhaustShader, quad, projection * editorCamera.getViewMatrix());

        // Post process
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);

        {
            postProcessShader.use();
            postProcessShader.setVec2("screenSize", glm::vec2((float)screenWidth, (float)screenHeight));
            // postProcessShader.setFloat("blurOffset", blurOffset);
            postProcessShader.setFloat("exposure", postProcess.m_exposure);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(postProcessShader.id, "renderedTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, postProcess.m_texture);

            glBindVertexArray(q_vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Shadowmap debug - should be called after post process
        shadowmapUI.drawShadowmap(textureArrayShader, screenWidth, screenHeight, q_vao);

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
