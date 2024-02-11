#include "../../src/enigine.h"

void setupTerrainTextures(ResourceManager *resourceManager, Terrain &terrain)
{
    std::string folders[] = {
        "Ground037_1K-JPG",
        "Ground052_1K-JPG",
        "Ground054_1K-JPG",
        "Rock030_1K-JPG",
        "Foam003_1K-JPG",
    };
    std::string files[] = {
        "_Color.jpg",
        "_NormalGL.jpg",
        "_AmbientOcclusion.jpg",
        "_Roughness.jpg",
        "_Displacement.jpg",
    };
    std::vector<Texture> terrainTextures;
    for (int i = 0; i < 5; i++)
    {
        std::vector<std::string> texturePaths;
        texturePaths.push_back("assets/terrain/pbr-texture-3/" + folders[0] + "/" + folders[0] + files[i]);
        texturePaths.push_back("assets/terrain/pbr-texture-3/" + folders[1] + "/" + folders[1] + files[i]);
        texturePaths.push_back("assets/terrain/pbr-texture-3/" + folders[2] + "/" + folders[2] + files[i]);
        texturePaths.push_back("assets/terrain/pbr-texture-3/" + folders[3] + "/" + folders[3] + files[i]);
        texturePaths.push_back("assets/terrain/pbr-texture-3/" + folders[4] + "/" + folders[4] + files[i]);
        terrainTextures.push_back(resourceManager->getTextureArray(texturePaths, true));
    }
    terrain.m_diffuseArray = terrainTextures[0];
    terrain.m_normalArray = terrainTextures[1];
    terrain.m_aoArray = terrainTextures[2];
    terrain.m_roughArray = terrainTextures[3];
    terrain.m_heightArray = terrainTextures[4];
}

int main()
{
    Enigine enigine;
    enigine.init();

    GLFWwindow *window = enigine.window;
    RenderManager *renderManager = enigine.renderManager;
    ResourceManager *resourceManager = enigine.resourceManager;
    ShaderManager *shaderManager = enigine.shaderManager;
    PhysicsWorld *physicsWorld = enigine.physicsWorld;
    DebugDrawer *debugDrawer = enigine.debugDrawer;
    UpdateManager *updateManager = enigine.updateManager;
    TaskManager *taskManager = enigine.taskManager;
    InputManager *inputManager = enigine.inputManager;
    SoundEngine *soundEngine = enigine.soundEngine;
    Camera *mainCamera = enigine.mainCamera;

    // debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    // characters
    NPCharacter npc1(renderManager, taskManager, resourceManager, physicsWorld, mainCamera);
    PCharacter character(shaderManager, renderManager, taskManager, soundEngine, window, resourceManager, physicsWorld, mainCamera);
    mainCamera->position = npc1.m_position + glm::vec3(0.f, -1.f, 10.f);

    std::vector<Character *> characters;
    characters.push_back(&character);
    characters.push_back(&npc1);

    character.m_npcList.push_back(&npc1);

    updateManager->add(&npc1);
    updateManager->add(&character);

    // vehicle
    Models models;
    models.carBody = resourceManager->getModel("assets/918/body.gltf");
    models.wheelBase = resourceManager->getModel("assets/918/wheel-base-1-LOD2.glb");
    models.doorFront = resourceManager->getModel("assets/918/door-front.gltf");
    models.smokeParticleModel = resourceManager->getModel("assets/car/smoke.obj");

    Model *collider = resourceManager->getModel("assets/918/collider.obj");
    eTransform bodyOffset;

    Vehicle vehicle(physicsWorld, VehicleType::coupe, collider, bodyOffset, glm::vec3(205.f, 2.f, 255.f));
    CarController car(window, shaderManager, renderManager, resourceManager, &vehicle, mainCamera, models, 2);

    float wheelSize[4] = {0.36f, 0.36f, 0.38f, 0.38f};
    glm::vec3 wheelPos[4] = {
        glm::vec3(0.8, 0.81f, 1.27),
        glm::vec3(-0.8, 0.81f, 1.27),
        glm::vec3(0.82, 0.85f, -1.43),
        glm::vec3(-0.82, 0.85f, -1.43)};

    for (int i = 0; i < 4; i++)
    {
        vehicle.setWheelPosition(i, wheelPos[i]);
        vehicle.setWheelRadius(i, wheelSize[i]);
        car.m_linkTireSmokes[i]->m_offset.setPosition(wheelPos[i] - glm::vec3(0.f, 0.7f, 0.f));
    }

    // door offset - when opened
    vehicle.setActiveDoorHingeOffsetFront(glm::vec3(0.8f, 1.17f, 0.8f), glm::vec3(0.6f, 0.0f, 0.5f));

    // TODO: single source
    car.m_linkDoors[0]->m_offsetActive.setPosition(glm::vec3(-0.2f, -0.8f, -0.67f));
    car.m_linkDoors[0]->m_offsetActive.setRotation(glm::quat(0.5f, 0.5f, 0.5f, 0.5f));
    vehicle.m_doors[0].rigidbodyOffset.setPosition(glm::vec3(0.8f, 0.67f, 0.2f));
    vehicle.m_doors[0].rigidbodyOffset.setRotation(glm::quat(0.5f, -0.5f, -0.5f, -0.5f));

    car.m_linkDoors[1]->m_offsetActive.setPosition(glm::vec3(-0.2f, 0.8f, -0.67f));
    car.m_linkDoors[1]->m_offsetActive.setRotation(glm::quat(0.5f, 0.5f, 0.5f, 0.5f));
    vehicle.m_doors[1].rigidbodyOffset.setPosition(glm::vec3(-0.8f, 0.67f, 0.2f));
    vehicle.m_doors[1].rigidbodyOffset.setRotation(glm::quat(0.5f, -0.5f, -0.5f, -0.5f));

    // TODO: cull face front because of mirrored
    car.m_linkDoors[1]->m_offsetActive.setScale(glm::vec3(-1.f, 1.f, 1.f));
    car.m_linkDoors[1]->m_offsetDeactive.setScale(glm::vec3(-1.f, 1.f, 1.f));

    car.m_linkWheels[0]->m_offset.setScale(glm::vec3(1.f, 1.12f, 1.12f));
    car.m_linkWheels[1]->m_offset.setScale(glm::vec3(1.f, 1.12f, 1.12f));
    car.m_linkWheels[2]->m_offset.setScale(glm::vec3(1.f, 1.18f, 1.18f));
    car.m_linkWheels[3]->m_offset.setScale(glm::vec3(1.f, 1.18f, 1.18f));

    // door collider
    glm::vec3 size(0.6, 0.15, 0.4);
    vehicle.m_doors[0].body->getCollisionShape()->setLocalScaling(BulletGLM::getBulletVec3(size));
    vehicle.m_doors[1].body->getCollisionShape()->setLocalScaling(BulletGLM::getBulletVec3(size));

    // exhaust
    car.m_linkExhausts[0]->m_offset.setPosition(glm::vec3(0.32f, 1.f, -1.02f));
    car.m_linkExhausts[0]->m_offset.setRotation(glm::quat(0.f, 1.f, 0.f, 0.f));
    car.m_linkExhausts[1]->m_offset.setPosition(glm::vec3(-0.32f, 1.f, -1.02f));
    car.m_linkExhausts[1]->m_offset.setRotation(glm::quat(0.f, 1.f, 0.f, 0.f));

    car.m_controlVehicle = true;
    car.m_followVehicle = true;
    inputManager->addKeyListener(std::bind(&CarController::keyListener, &car,
                                           std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    // TODO: before character - parent transform - ?
    updateManager->add(&car);

    VehicleUI *vehicleUI = new VehicleUI(&car, &vehicle);
    enigine.rootUI->m_uiList.push_back(vehicleUI);

    // create the scene
    eTransform transform;
    transform.setPosition(glm::vec3(103.f, 1.8f, 260.f));
    transform.setScale(glm::vec3(.7f));
    Model &shelter = *resourceManager->getModel("assets/gltf/shelter1.glb");
    renderManager->addSource(RenderSourceBuilder()
                                 .setTransform(transform)
                                 .setModel(&shelter)
                                 .build());

    transform.setPosition(glm::vec3(112.f, 0.f, 233.f));
    transform.setScale(glm::vec3(.7f));
    Model &tower = *resourceManager->getModel("assets/gltf/old-water-tower.glb");
    renderManager->addSource(RenderSourceBuilder()
                                 .setTransform(transform)
                                 .setModel(&tower)
                                 .build());

    Terrain terrain(renderManager, resourceManager, shaderManager, physicsWorld, "/assets/images/test-5.png", -1.0f, 517.0f, 6.0f);
    setupTerrainTextures(resourceManager, terrain);

    TerrainUI *terrainUI = new TerrainUI(&terrain);
    enigine.rootUI->m_uiList.push_back(terrainUI);

    // TODO: update
    // terrain.m_playerPos = character.m_position;
    renderManager->addRenderable(&terrain);

    // environment map
    std::string path = resourceManager->m_executablePath + "/assets/hdr_skybox/skybox-7.hdr";
    TextureParams textureParams;
    textureParams.dataType = TextureDataType::Float32;
    Texture envTexture = resourceManager->textureFromFile(textureParams, path, path);
    renderManager->updateEnvironmentTexture(envTexture);

    enigine.start();

    return 0;
}
