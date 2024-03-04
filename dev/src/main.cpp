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

    // environment map
    std::string path = resourceManager->m_executablePath + "/assets/hdr_skybox/skybox-7.hdr";
    TextureParams textureParams;
    textureParams.dataType = TextureDataType::Float32;
    Texture envTexture = resourceManager->textureFromFile(textureParams, path, path);
    renderManager->updateEnvironmentTexture(envTexture);

    enigine.start();

    return 0;
}
