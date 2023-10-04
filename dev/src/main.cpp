#include "../../src/enigine.h"

int main()
{
    Enigine enigine;
    enigine.init();

    GLFWwindow *window = enigine.window;
    RenderManager *renderManager = enigine.renderManager;
    ResourceManager *resourceManager = enigine.resourceManager;
    ShaderManager *shaderManager = enigine.shaderManager;
    PhysicsWorld *physicsWorld = enigine.physicsWorld;
    UpdateManager *updateManager = enigine.updateManager;
    TaskManager *taskManager = enigine.taskManager;
    SoundEngine *soundEngine = enigine.soundEngine;
    Camera *mainCamera = enigine.mainCamera;

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
    CarController car(window, shaderManager, renderManager, physicsWorld, resourceManager, mainCamera, character.m_position + glm::vec3(10.f, 5.f, 10.f));
    // TODO: input manager
    glfwSetWindowUserPointer(window, &car);
    glfwSetKeyCallback(window, car.staticKeyCallback);

    // TODO: before character - parent transform - ?
    updateManager->add(&car);

    // create the scene
    eTransform transform;
    transform.setPosition(glm::vec3(103.f, 1.8f, 260.f));
    Model &shelter = *resourceManager->getModel("assets/gltf/shelter1.glb");
    renderManager->addSource(RenderSourceBuilder()
                                 .setTransform(transform)
                                 .setAoRoughMetalMap(true)
                                 .setModel(&shelter)
                                 .build());

    transform.setPosition(glm::vec3(112.f, 18.2f, 233.f));
    transform.setScale(glm::vec3(.1f, .1f, .1f));
    Model &tower = *resourceManager->getModel("assets/gltf/old-water-tower.glb", false);
    renderManager->addSource(RenderSourceBuilder()
                                 .setTransform(transform)
                                 .setAoRoughMetalMap(true)
                                 .setModel(&tower)
                                 .build());

    Terrain terrain(renderManager, resourceManager, shaderManager, physicsWorld, "/assets/images/test-5.png", -1.0f, 517.0f, 6.0f, true);
    // TODO: update
    // terrain.m_playerPos = character.m_position;
    renderManager->addRenderable(&terrain);

    enigine.start();

    return 0;
}
