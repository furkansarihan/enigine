#include <iostream>
#include <ctime>
#include <bit>
#include <mach/mach.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "external/imgui/imgui.h"

#include "shader/shader.h"
#include "file_manager/file_manager.h"
#include "camera/camera.h"
#include "model/model.h"
#include "sound_engine/sound_engine.h"
#include "physics_world/physics_world.h"
#include "physics_world/debug_drawer/debug_drawer.h"
#include "terrain/terrain.h"
#include "vehicle/vehicle.h"

#include "external/stb_image/stb_image.h"

enum ProjectionMode
{
    Perspective,
    Ortho,
};

bool firstMove = true;
float lastX;
float lastY;

// Sphere
glm::vec3 spherePosition = glm::vec3(80.0f, 2.0f, 80.0f);
// Light
glm::vec3 lightPosition = glm::vec3(1.8f, 1.2f, -0.7f);
static float lightColor[3] = {0.1f, 0.1f, 0.1f};
static float ambientColor[3] = {0.1f, 0.1f, 0.1f};
static float specularColor[3] = {0.3f, 0.3f, 0.3f};
static float lightPower = 10.0;
// Camera
ProjectionMode projectionMode = ProjectionMode::Perspective;
static float near = 0.1;
static float far = 10000.0;
static int speed = 4;
static int textureIndex = 0;
static float degree = 45;
static float scaleOrtho = 1.0;
static bool followVehicle = false;
static bool drawShadowmap = true;
static float bias = 0.005;
glm::vec3 followDistance = glm::vec3(10.0, 4.5, -10.0);
// Shaders
Shader normalShader;
Shader simpleShader;
Shader depthShader;
Shader simpleShadow;
Shader terrainShader;
Shader terrainShadow;
Shader lineShader;
Shader textureShader;
// Physics
btRigidBody *sphereBody;
// System Monitor
task_basic_info t_info;

static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void printStartInfo()
{
    // current date/time based on current system
    time_t now = time(0);
    char *dateTime = ctime(&now);
    // version, format -> x.xx.xxx
    std::string version("000001");

    std::cout << "enigine_version: " << version << std::endl;
    std::cout << "cpp_version: " << __cplusplus << std::endl;
    std::cout << "started_at: " << dateTime << std::endl;
}

// TODO: support other platforms than macOS
static void refreshSystemMonitor()
{
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    task_info(mach_task_self(),
              TASK_BASIC_INFO, (task_info_t)&t_info,
              &t_info_count);
}

void initShaders()
{
    normalShader.init(FileManager::read("../src/assets/shaders/normal-shader.vs"), FileManager::read("../src/assets/shaders/normal-shader.fs"));
    simpleShader.init(FileManager::read("../src/assets/shaders/simple-shader.vs"), FileManager::read("../src/assets/shaders/simple-shader.fs"));
    depthShader.init(FileManager::read("../src/assets/shaders/simple-shader.vs"), FileManager::read("../src/assets/shaders/depth-shader.fs"));
    simpleShadow.init(FileManager::read("../src/assets/shaders/simple-shadow.vs"), FileManager::read("../src/assets/shaders/simple-shadow.fs"));
    terrainShader.init(FileManager::read("../src/assets/shaders/terrain-shader.vs"), FileManager::read("../src/assets/shaders/terrain-shader.fs"));
    terrainShadow.init(FileManager::read("../src/assets/shaders/terrain-shadow.vs"), FileManager::read("../src/assets/shaders/terrain-shadow.fs"));
    lineShader.init(FileManager::read("../src/assets/shaders/line-shader.vs"), FileManager::read("../src/assets/shaders/line-shader.fs"));
    textureShader.init(FileManager::read("../src/assets/shaders/simple-texture.vs"), FileManager::read("../src/assets/shaders/simple-texture.fs"));
}

void processCameraInput(GLFWwindow *window, Camera *editorCamera, float deltaTime)
{
    deltaTime = deltaTime * speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        editorCamera->processKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        editorCamera->processKeyboard(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        editorCamera->processKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        editorCamera->processKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (firstMove)
        {
            lastX = xpos;
            lastY = ypos;
            firstMove = false;
        }
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;
        editorCamera->processMouseMovement(xoffset, yoffset, true);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
    {
        firstMove = true;
    }
}

static void showOverlay(Camera *editorCamera, Vehicle *vehicle, SoundEngine *soundEngine, Terrain *terrain, DebugDrawer *debugDrawer, SoundSource soundSource, float deltaTime, bool *p_open)
{
    static int corner = 0;
    ImGuiIO &io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x : 0, (corner & 2) ? io.DisplaySize.y : 0);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("overlay", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f, %.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("RAM: %d", (int)t_info.resident_size);
        ImGui::Separator();
        ImGui::Text("Camera");
        ImGui::Text("position: (%.1f, %.1f, %.1f)", editorCamera->position.x, editorCamera->position.y, editorCamera->position.z);
        ImGui::Text("pitch: %.1f", editorCamera->pitch);
        ImGui::Text("yaw: %.1f", editorCamera->yaw);
        ImGui::DragFloat("near", &near, 10.0f);
        ImGui::DragFloat("far", &far, 10.0f);
        ImGui::DragFloat("degree", &degree, 0.01f);
        ImGui::DragInt("speed", &speed, 10);
        ImGui::DragInt("textureIndex", &textureIndex, 1);
        ImGui::DragFloat("scaleOrtho", &scaleOrtho, 0.1f);
        ImGui::DragFloat("bias", &bias, 0.001f);
        ImGui::Checkbox("followVehicle", &followVehicle);
        ImGui::Checkbox("drawShadowmap", &drawShadowmap);
        if (ImGui::RadioButton("perspective", projectionMode == ProjectionMode::Perspective))
        {
            projectionMode = ProjectionMode::Perspective;
        }
        if (ImGui::RadioButton("ortho", projectionMode == ProjectionMode::Ortho))
        {
            projectionMode = ProjectionMode::Ortho;
        }
        if (ImGui::Button("jump-position-1"))
        {
            editorCamera->position = glm::vec3(2584.9f, 87.3f, 890.4f);
            editorCamera->pitch = 0.2f;
            editorCamera->yaw = 137.2f;
            lightPosition = glm::vec3(0.0f, 238.8f, 14.3f);
        }
        if (ImGui::Button("jump-position-2"))
        {
            editorCamera->position = glm::vec3(1024.0f, 137.0f, 2502.0f);
            editorCamera->pitch = -3.5f;
            editorCamera->yaw = 315.7f;
            lightPosition = glm::vec3(0.0f, 200.0f, 0.0f);
        }
        ImGui::Separator();
        ImGui::Text("Light");
        ImGui::DragFloat("X", &lightPosition.x, 0.01f);
        ImGui::DragFloat("Y", &lightPosition.y, 0.01);
        ImGui::DragFloat("Z", &lightPosition.z, 0.01);
        ImGui::DragFloat("power", &lightPower, 0.1);
        ImGui::ColorEdit3("lightColor", lightColor);
        ImGui::ColorEdit3("ambientColor", ambientColor);
        ImGui::ColorEdit3("specularColor", specularColor);
        if (ImGui::Button("refresh shaders"))
        {
            initShaders();
        }
        ImGui::Separator();
        ImGui::Text("Vehicle");
        ImGui::Text("gVehicleSteering = %f", vehicle->gVehicleSteering);
        ImGui::DragFloat("steeringClamp", &vehicle->steeringClamp, 0.1f);
        ImGui::DragFloat("maxEngineForce", &vehicle->maxEngineForce, 2.0f);
        ImGui::DragFloat("accelerationVelocity", &vehicle->accelerationVelocity, 2.0f);
        ImGui::DragFloat("decreaseVelocity", &vehicle->decreaseVelocity, 2.0f);
        ImGui::DragFloat("breakingVelocity", &vehicle->breakingVelocity, 2.0f);
        ImGui::DragFloat("steeringIncrement", &vehicle->steeringIncrement, 0.2f);
        ImGui::DragFloat("steeringVelocity", &vehicle->steeringVelocity, 10.0f);
        if (ImGui::DragFloat("lowerLimit", &vehicle->lowerLimit, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheels[i]->setLimit(2, vehicle->lowerLimit, vehicle->upperLimit);
            }
        }
        if (ImGui::DragFloat("upperLimit", &vehicle->upperLimit, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheels[i]->setLimit(2, vehicle->lowerLimit, vehicle->upperLimit);
            }
        }
        if (ImGui::DragFloat("wheel damping", &vehicle->damping, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheelBodies[i]->setDamping(vehicle->damping, vehicle->damping);
            }
        }
        if (ImGui::DragFloat("wheel friction", &vehicle->friction, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheelBodies[i]->setFriction(vehicle->friction);
            }
        }
        if (ImGui::DragFloat("wheel stifness", &vehicle->stifness, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheels[i]->setStiffness(2, vehicle->stifness);
            }
        }
        if (ImGui::DragFloat("wheel damping", &vehicle->wheelDamping, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheels[i]->setDamping(2, vehicle->wheelDamping);
            }
        }
        if (ImGui::DragFloat("wheel bounce", &vehicle->bounce, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheels[i]->setBounce(2, vehicle->bounce);
            }
        }
        float restitution = vehicle->wheelBodies[0]->getRestitution();
        if (ImGui::DragFloat("wheel restitution", &restitution, 0.1f))
        {
            for (int i = 0; i < 4; i++)
            {
                vehicle->wheelBodies[i]->setRestitution(restitution);
            }
        }
        float mass = vehicle->m_carChassis->getMass();
        if (ImGui::DragFloat("mass", &mass, 1.0f, 1, 10000))
        {
            btVector3 interia;
            vehicle->m_carChassis->getCollisionShape()->calculateLocalInertia(mass, interia);
            vehicle->m_carChassis->setMassProps(mass, interia);
        }
        ImGui::Separator();
        ImGui::Text("Audio");
        ALint state = soundEngine->getSourceState(soundSource);
        if (state == AL_PLAYING)
        {
            if (ImGui::Button("state: playing"))
            {
                soundEngine->pauseSource(soundSource);
            }
        }
        else if (state == AL_PAUSED)
        {
            if (ImGui::Button("state: paused"))
            {
                soundEngine->playSource(soundSource);
            }
        }
        else if (state == AL_STOPPED)
        {
            if (ImGui::Button("state: stopped"))
            {
                soundEngine->playSource(soundSource);
            }
        }
        else
        {
            ImGui::Text("state: unknown");
        }
        ImGui::SameLine();
        if (ImGui::Button("reset"))
        {
            soundEngine->setSourceGain(soundSource, 1.0f);
            soundEngine->setSourcePitch(soundSource, 1.0f);
        }
        ALfloat gain = soundEngine->getSourceGain(soundSource);
        if (ImGui::SliderFloat("gain", &gain, 0.0f, 1.0f, "%.3f"))
        {
            soundEngine->setSourceGain(soundSource, gain);
        }
        ALfloat pitch = soundEngine->getSourcePitch(soundSource);
        if (ImGui::SliderFloat("pitch", &pitch, 0.5f, 2.0f, "%.3f"))
        {
            soundEngine->setSourcePitch(soundSource, pitch);
        }
        ALint looping = soundEngine->getSourceLooping(soundSource);
        bool isLooping = looping == AL_TRUE;
        if (ImGui::Checkbox("looping", &isLooping))
        {
            soundEngine->setSourceLooping(soundSource, looping ? AL_FALSE : AL_TRUE);
        }
        ImGui::Separator();
        ImGui::Text("Physics");
        bool debugEnabled = debugDrawer->getDebugMode();
        if (ImGui::Checkbox("wireframe", &debugEnabled))
        {
            debugDrawer->setDebugMode(debugEnabled ? btIDebugDraw::DBG_DrawAabb : btIDebugDraw::DBG_NoDebug);
        }
        int lines = debugDrawer->getLines().size();
        ImGui::DragInt("lines", &lines);
        ImGui::Separator();
        ImGui::Text("Sphere");
        // ImGui::Text("position: (%.1f, %.1f, %.1f)", spherePosition.x, spherePosition.y, spherePosition.z);
        float sX = sphereBody->getWorldTransform().getOrigin().getX();
        float sY = sphereBody->getWorldTransform().getOrigin().getY();
        float sZ = sphereBody->getWorldTransform().getOrigin().getZ();
        if (ImGui::DragFloat("sX", &sX, 0.1f))
        {
            sphereBody->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
            sphereBody->setLinearVelocity(btVector3(0, 0, 0));
        }
        if (ImGui::DragFloat("sY", &sY, 0.1))
        {
            sphereBody->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
            sphereBody->setLinearVelocity(btVector3(0, 0, 0));
        }
        if (ImGui::DragFloat("sZ", &sZ, 0.1))
        {
            sphereBody->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
            sphereBody->setLinearVelocity(btVector3(0, 0, 0));
        }
        if (ImGui::Button("jump"))
        {
            sphereBody->setActivationState(1);
            sphereBody->setLinearVelocity(btVector3(0, 0, 0));
            sphereBody->setAngularVelocity(btVector3(0, 0, 0));
            sphereBody->applyCentralForce(btVector3(1000, 1000, 1000));
        }
        float scaleX = terrain->terrainBody->getCollisionShape()->getLocalScaling().getX();
        float scaleY = terrain->terrainBody->getCollisionShape()->getLocalScaling().getY();
        float scaleZ = terrain->terrainBody->getCollisionShape()->getLocalScaling().getZ();
        if (ImGui::DragFloat("scaleY", &scaleY, 10.0))
        {
            terrain->terrainBody->getCollisionShape()->setLocalScaling(btVector3(scaleX, scaleY, scaleZ));
        }
        ImGui::Separator();
        ImGui::Text("Terrain");
        ImGui::Checkbox("wirewrame", &terrain->wireframe);
        ImGui::DragInt("level", &terrain->level);
        ImGui::DragFloat("scale factor", &terrain->scaleFactor, 0.05f);
        ImGui::DragFloat("fogMaxDist", &terrain->fogMaxDist, 100.0f);
        ImGui::DragFloat("fogMinDist", &terrain->fogMinDist, 100.0f);
        ImGui::ColorEdit4("fogColor", &terrain->fogColor[0]);
        ImGui::Text("center");
        ImGui::DragFloat("terrainCenter-X", &terrain->terrainCenter.x, 1.0f);
        ImGui::DragFloat("terrainCenter-Z", &terrain->terrainCenter.z, 1.0);
        ImGui::DragFloat("uvOffset-X", &terrain->uvOffset.x, 0.001f);
        ImGui::DragFloat("uvOffset-Y", &terrain->uvOffset.y, 0.001);
        ImGui::DragFloat("alphaOffset-X", &terrain->alphaOffset.x, 1.000f);
        ImGui::DragFloat("alphaOffset-Y", &terrain->alphaOffset.y, 1.000);
        ImGui::DragFloat("oneOverWidth", &terrain->oneOverWidth, 0.01f);
        float trestitution = terrain->terrainBody->getRestitution();
        if (ImGui::DragFloat("terrain restitution", &trestitution, 0.1f))
        {
            terrain->terrainBody->setRestitution(trestitution);
        }
    }
    ImGui::End();
}

void createQuad(unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
{
    // TODO: fix stretch - aspect ratio
    float size = 0.5f;

    float topLeftX = 1 - size;
    float topLeftY = -1 + size;

    float topRightX = topLeftX + size;
    float topRightY = topLeftY;
    float bottomLeftX = topLeftX;
    float bottomLeftY = topLeftY - size;
    float bottomRightX = topRightX;
    float bottomRightY = bottomLeftY;

    // std::cout << "topLeftX:" << topLeftX << std::endl;
    // std::cout << "topLeftY:" << topLeftY << std::endl;
    // std::cout << "topRightX:" << topRightX << std::endl;
    // std::cout << "topRightY:" << topRightY << std::endl;
    // std::cout << "bottomLeftX:" << bottomLeftX << std::endl;
    // std::cout << "bottomLeftY:" << bottomLeftY << std::endl;
    // std::cout << "bottomRightX:" << bottomRightX << std::endl;
    // std::cout << "bottomRightY:" << bottomRightY << std::endl;

    float quad_vertices[] = {
        // top left
        topLeftX, topLeftY,
        0.0f, 1.0f,
        // bottom left
        bottomLeftX, bottomLeftY,
        0.0f, 0.0f,
        // bottom right
        bottomRightX, bottomRightY,
        1.0f, 0.0f,
        // top right
        topRightX, topRightY,
        1.0f, 1.0f};

    unsigned int quad_indices[] = {
        0, 1, 2, 0, 3, 2};
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int main(int argc, char **argv)
{
    printStartInfo();

    // Setup window
    glfwSetErrorCallback(glfwErrorCallback);
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
    GLFWwindow *window = glfwCreateWindow(1280, 720, "enigine", NULL, NULL);
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
    glDepthFunc(GL_LESS);
    // Trasparency
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Init OpenAL
    SoundEngine soundEngine;

    if (!soundEngine.init())
    {
        fprintf(stderr, "Failed to initialize OpenAL!\n");
        return 0;
    }

    soundEngine.setListenerPosition(4.0f, 4.0f, 4.0f);

    SoundSource soundSource;

    try
    {
        soundSource = soundEngine.loadSource("assets/sounds/rain-loop-1648m.wav");
    }
    catch (const char *e)
    {
        std::cerr << e << std::endl;
        return 0;
    }

    soundEngine.setSourcePosition(soundSource, spherePosition.x, spherePosition.y, spherePosition.z);
    soundEngine.playSource(soundSource);

    // Init Physics
    PhysicsWorld physicsWorld;

    DebugDrawer debugDrawer;
    debugDrawer.setDebugMode(btIDebugDraw::DBG_NoDebug);
    physicsWorld.dynamicsWorld->setDebugDrawer(&debugDrawer);

    btRigidBody *groundBody = physicsWorld.createBox(0, btVector3(10.0f, 10.0f, 10.0f), btVector3(0, -10, 0));
    // set ground bouncy
    groundBody->setRestitution(0.5f);
    sphereBody = physicsWorld.createSphere(1.0f, 1.0f, btVector3(spherePosition.x, spherePosition.y, spherePosition.z));

    // Physics debug drawer objects
    unsigned int vbo, vao, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    // Create geometries
    Model cube("assets/models/cube.obj");
    Model sphere("assets/models/sphere.obj");
    Model wheel("assets/models/wheel.obj");
    Model cylinder("assets/models/cylinder.obj");

    // Init shaders
    initShaders();

    // Camera
    Camera editorCamera(glm::vec3(25.0f, 18.0f, 25.0f), glm::vec3(0.0f, 1.0f, 0.0f), 224.0f, -22.0f);

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

    bool show_overlay = false;

    // Terrain
    Terrain terrain(&physicsWorld);

    // Vehicle
    Vehicle vehicle(&physicsWorld, btVector3(1700, 10, 1750));
    glfwSetWindowUserPointer(window, &vehicle);
    glfwSetKeyCallback(window, vehicle.staticKeyCallback);

    // Shadowmap
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    float shadowmapSize = 1024;

    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    // depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowmapSize, shadowmapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // for sample2DShadow
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // Set "renderedTexture" as our colour attachement #0
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
    // depth
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderedTexture, 0);

    // Set the list of draw buffers.
    // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    // glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    // depth
    glDrawBuffer(GL_NONE);

    // Always check that our framebuffer is ok
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Framebuffer creation error!\n");
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Shadowmap display quad
    unsigned int q_vbo, q_vao, q_ebo;
    createQuad(q_vbo, q_vao, q_ebo);

    while (!glfwWindowShouldClose(window))
    {
        // System monitor
        refreshSystemMonitor();

        // Calculate deltaTime
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll events
        glfwPollEvents();

        // Process input
        processCameraInput(window, &editorCamera, deltaTime);

        // Update Physics
        physicsWorld.dynamicsWorld->stepSimulation(deltaTime, 2);
        if (physicsWorld.dynamicsWorld->getConstraintSolver()->getSolverType() == BT_MLCP_SOLVER)
        {
            btMLCPSolver *sol = (btMLCPSolver *)physicsWorld.dynamicsWorld->getConstraintSolver();
            int numFallbacks = sol->getNumFallbacks();
            if (numFallbacks)
            {
                static int totalFailures = 0;
                totalFailures += numFallbacks;
                printf("MLCP solver failed %d times, falling back to btSequentialImpulseSolver (SI)\n", totalFailures);
            }
            sol->setNumFallbacks(0);
        }

        // Update Debug Drawer
        debugDrawer.getLines().clear();
        physicsWorld.dynamicsWorld->debugDrawWorld();

        // Syncronize with Physics
        if (sphereBody && sphereBody->getMotionState())
        {
            btTransform trans;
            sphereBody->getMotionState()->getWorldTransform(trans);
            spherePosition = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            soundEngine.setSourcePosition(soundSource, spherePosition.x, spherePosition.y, spherePosition.z);
        }

        if (followVehicle && vehicle.m_carChassis && vehicle.m_carChassis->getMotionState())
        {
            btTransform trans;
            vehicle.m_carChassis->getMotionState()->getWorldTransform(trans);
            glm::vec3 vehiclePosition = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            editorCamera.position = vehiclePosition + followDistance;
        }

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
        glm::mat4 projection;

        if (projectionMode == ProjectionMode::Perspective)
        {
            projection = glm::perspective(degree, (float)screenWidth / (float)screenHeight, near, far);
        }
        else
        {
            projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, near, far);
            projection = glm::scale(projection, glm::vec3(scaleOrtho, scaleOrtho, 1));
        }

        // Render geometries
        glm::mat4 model = glm::translate(glm::mat4(1.0f), spherePosition);
        glm::mat4 mvp = projection * editorCamera.getViewMatrix() * model; // Model-View-Projection matrix
        glm::mat4 mv = editorCamera.getViewMatrix() * model;
        glm::mat3 ModelView3x3Matrix = glm::mat3(mv);

        normalShader.use();
        normalShader.setMat4("MVP", mvp);
        normalShader.setMat4("M", model);
        normalShader.setMat4("V", editorCamera.getViewMatrix());
        normalShader.setMat3("MV3x3", ModelView3x3Matrix);
        normalShader.setVec3("LightPosition_worldspace", lightPosition);
        normalShader.setVec3("LightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
        normalShader.setFloat("LightPower", lightPower);
        sphere.draw(normalShader);

        // Vehicle
        vehicle.update(window, deltaTime);

        // Render vehicle
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        simpleShader.use();
        glm::mat4 transform;
        vehicle.m_carChassis->getWorldTransform().getOpenGLMatrix((btScalar *)&transform);
        transform = glm::translate(transform, glm::vec3(0, 1, 0));
        transform = glm::scale(transform, glm::vec3(0.8f, 0.3f, 2.8f));
        simpleShader.setMat4("MVP", projection * editorCamera.getViewMatrix() * transform);
        simpleShader.setVec4("DiffuseColor", glm::vec4(1.0, 1.0, 1.0, 1.0f));
        cube.draw(simpleShader);

        for (int i = 0; i < 4; i++)
        {
            btRigidBody body = vehicle.wheels[i]->getRigidBodyB();
            glm::mat4 transform;
            body.getWorldTransform().getOpenGLMatrix((btScalar *)&transform);
            transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));
            simpleShader.setMat4("MVP", projection * editorCamera.getViewMatrix() * transform);
            simpleShader.setVec4("DiffuseColor", glm::vec4(0.0, 1.0, 1.0, 1.0f));
            wheel.draw(simpleShader);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Build shadowmap
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        GLfloat clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
        GLfloat clearDepth[] = {1.0f};
        GLint clearStencil[] = {0};

        glClearBufferfv(GL_COLOR, 0, clearColor);
        glClearBufferfv(GL_DEPTH, 0, clearDepth);
        glClearBufferiv(GL_STENCIL, 0, clearStencil);
        glViewport(0, 0, shadowmapSize, shadowmapSize);

        // glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);
        glm::mat4 depthProjectionMatrix = glm::ortho<float>(-20, 20, -20, 20, -20, 20);
        glm::mat4 depthViewMatrix = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 depthVP = depthProjectionMatrix * depthViewMatrix;
        // glm::mat4 depthVP = projection * editorCamera.getViewMatrix();

        // TODO: cast terrain shadows
        // terrain.draw(terrainShadow, editorCamera.position, lightPosition, glm::vec3(lightColor[0], lightColor[1], lightColor[2]),
        //     lightPower, depthViewMatrix, depthProjectionMatrix, depthVP, near, far, 0);

        {
            // We don't use bias in the shader, but instead we draw back faces,
            // which are already separated from the front faces by a small distance
            // (if your geometry is made this way)
            // glCullFace(GL_FRONT);

            // Draw objects
            glm::vec4 objectColor(0.6f, 0.6f, 0.6f, 1.0f);

            // wall
            glm::vec3 position = glm::vec3(0, 1.25, -9);
            glm::vec3 scale = glm::vec3(10.0f, 4.0f, 1.0f);
            glm::mat4 objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.use();
            depthShader.setBool("DrawDepth", false);
            depthShader.setMat4("MVP", depthVP * objectModel);
            depthShader.setVec4("DiffuseColor", objectColor);
            cube.draw(depthShader);

            // ball
            position = glm::vec3(0, 2, -1);
            scale = glm::vec3(2.0f, 2.0f, 2.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.use();
            depthShader.setBool("DrawDepth", false);
            depthShader.setMat4("MVP", depthVP * objectModel);
            depthShader.setVec4("DiffuseColor", objectColor);
            sphere.draw(depthShader);

            // stick
            position = glm::vec3(1, 4, 2);
            scale = glm::vec3(4.0f, 1.0f, 1.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.setMat4("MVP", depthVP * objectModel);
            depthShader.setVec4("DiffuseColor", objectColor);
            cylinder.draw(depthShader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // glCullFace(GL_BACK);
        }

        glViewport(0, 0, screenWidth, screenHeight);

        // Draw shadowmap
        if (drawShadowmap)
        {
            textureShader.use();

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(textureShader.id, "renderedTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, renderedTexture);

            glBindVertexArray(q_vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Render scene
        glm::mat4 biasMatrix(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0);
        glm::mat4 depthBiasVP = biasMatrix * depthVP;

        terrain.draw(terrainShader, editorCamera.position, lightPosition, glm::vec3(lightColor[0], lightColor[1], lightColor[2]),
                     lightPower, editorCamera.getViewMatrix(), projection, depthBiasVP, near, far, renderedTexture);

        // draw objects
        {
            // Draw objects
            glm::vec3 objectColor(0.6f, 0.6f, 0.6f);

            simpleShadow.use();
            simpleShadow.setFloat("biasMult", bias);
            simpleShadow.setVec3("AmbientColor", glm::vec3(ambientColor[0], ambientColor[1], ambientColor[2]));
            simpleShadow.setVec3("DiffuseColor", objectColor);
            simpleShadow.setVec3("SpecularColor", glm::vec3(specularColor[0], specularColor[1], specularColor[2]));
            simpleShadow.setVec3("LightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
            simpleShadow.setFloat("LightPower", lightPower);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(simpleShadow.id, "ShadowMap"), 0);
            glBindTexture(GL_TEXTURE_2D, renderedTexture);

            // wall
            glm::vec3 position = glm::vec3(0, 1.25, -9);
            glm::vec3 scale = glm::vec3(10.0f, 4.0f, 1.0f);
            glm::mat4 objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("DepthBiasMVP", depthBiasVP * objectModel);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
            cube.draw(simpleShadow);

            // ball
            position = glm::vec3(0, 2, -1);
            scale = glm::vec3(2.0f, 2.0f, 2.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("DepthBiasMVP", depthBiasVP * objectModel);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
            sphere.draw(simpleShadow);

            // stick
            position = glm::vec3(1, 4, 2);
            scale = glm::vec3(4.0f, 1.0f, 1.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("DepthBiasMVP", depthBiasVP * objectModel);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
            cylinder.draw(simpleShadow);
        }

        // Draw light source
        mvp = projection * editorCamera.getViewMatrix() * glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)), lightPosition);
        simpleShader.use();
        simpleShader.setMat4("MVP", mvp);
        simpleShader.setVec4("DiffuseColor", glm::vec4(lightColor[0], lightColor[1], lightColor[2], 1.0f));
        cube.draw(simpleShader);

        // Draw physics debug lines
        std::vector<DebugDrawer::Line> &lines = debugDrawer.getLines();
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        unsigned int indexI = 0;

        for (std::vector<DebugDrawer::Line>::iterator it = lines.begin(); it != lines.end(); it++)
        {
            DebugDrawer::Line l = (*it);
            vertices.push_back(l.a.x);
            vertices.push_back(l.a.y);
            vertices.push_back(l.a.z);

            vertices.push_back(l.color.x);
            vertices.push_back(l.color.y);
            vertices.push_back(l.color.z);

            vertices.push_back(l.b.x);
            vertices.push_back(l.b.y);
            vertices.push_back(l.b.z);

            vertices.push_back(l.color.x);
            vertices.push_back(l.color.y);
            vertices.push_back(l.color.z);

            indices.push_back(indexI);
            indices.push_back(indexI + 1);
            indexI += 2;
        }

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

        mvp = projection * editorCamera.getViewMatrix() * glm::mat4(1.0f);
        lineShader.use();
        lineShader.setMat4("MVP", mvp);

        glBindVertexArray(vao);
        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        showOverlay(&editorCamera, &vehicle, &soundEngine, &terrain, &debugDrawer, soundSource, deltaTime, &show_overlay);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup OpenAL
    soundEngine.deleteSource(soundSource);

    // Cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // Cleanup glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
