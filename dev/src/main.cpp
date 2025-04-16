#include "enigine.h"

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
    InputManager *inputManager = enigine.inputManager;
    SoundEngine *soundEngine = enigine.soundEngine;
    Camera *mainCamera = enigine.mainCamera;

    // set environment map
    std::string path = resourceManager->m_executablePath + "/assets/hdris/qwantani_puresky.hdr";
    TextureParams textureParams;
    textureParams.dataType = TextureDataType::Float32;
    Texture *envTexture = resourceManager->textureFromFile(textureParams, path, path);
    renderManager->updateEnvironmentTexture(envTexture);
    renderManager->m_shadowManager->m_lightPos = glm::normalize(glm::vec3(0.693f, 0.51f, 0.51f));

    // add render source
    std::string modelPath = resourceManager->m_executablePath + "/assets/models/rim.glb";
    Model *rimModel = resourceManager->getModelFullPath(modelPath);
    eTransform transform;
    transform.setScale(glm::vec3(2.f));
    RenderSource *rimSource = RenderSourceBuilder()
                                  .setTransform(transform)
                                  .setModel(rimModel)
                                  .build();
    renderManager->addSource(rimSource);

    // update camera
    mainCamera->position = glm::vec3(-2.f, 0.f, 2.f);
    mainCamera->front = glm::vec3(0.707f, 0.f, -0.707f);

    enigine.start();

    return 0;
}
