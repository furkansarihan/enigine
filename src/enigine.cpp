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

#include "external/stb_image/stb_image.h"

bool firstMove = true;
float lastX;
float lastY;

// Sphere
glm::vec3 spherePosition = glm::vec3(80.0f, 200.0f, 80.0f);
// Light
glm::vec3 lightPosition = glm::vec3(0.0f, 200.0f, 0.0f);
static float lightColor[3] = {1.0f, 1.0f, 0.9f};
static float lightPower = 70.0;
// Camera
static float far = 10000.0;
static int speed = 20;
static bool wireframe = false;
// Terrain
glm::vec3 terrainCenter = glm::vec3(0.0f, 0.0f, 0.0f);
static int level = 9;
static float scaleFactor = 255.0f;
static float fogMaxDist = 6600.0f;
static float fogMinDist = 1000.0f;
static float fogColor[3] = {0.46f, 0.71f, 0.98f};
glm::vec2 uvOffset = glm::vec2(0.0f, 0.0f);
glm::vec2 alphaOffset = glm::vec2(0.0f, 0.0f);
static float oneOverWidth = 1.5f;
static float rotate = 0.0f;
// Physics
btRigidBody *sphereBody;
btRigidBody *terrainBody;
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
static void refreshSystemMonitor() {
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    task_info(mach_task_self(),
        TASK_BASIC_INFO, (task_info_t)&t_info,
        &t_info_count);
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

static void showOverlay(Camera *editorCamera, SoundEngine *soundEngine, DebugDrawer *debugDrawer, SoundSource soundSource, float deltaTime, bool *p_open)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImGuiIO &io = ImGui::GetIO();
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
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
        ImGui::DragFloat("far", &far, 10.0f);
        ImGui::DragInt("speed", &speed, 10);
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
        float x = lightPosition.x;
        float y = lightPosition.y;
        float z = lightPosition.z;
        if (ImGui::DragFloat("X", &x, 0.1f))
        {
            lightPosition = glm::vec3(x, y, z);
        }
        if (ImGui::DragFloat("Y", &y, 0.1))
        {
            lightPosition = glm::vec3(x, y, z);
        }
        if (ImGui::DragFloat("Z", &z, 0.1))
        {
            lightPosition = glm::vec3(x, y, z);
        }
        ImGui::DragFloat("power", &lightPower, 0.1);
        ImGui::ColorEdit3("color", lightColor);
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
        float scaleX = terrainBody->getCollisionShape()->getLocalScaling().getX();
        float scaleY = terrainBody->getCollisionShape()->getLocalScaling().getY();
        float scaleZ = terrainBody->getCollisionShape()->getLocalScaling().getZ();
        if (ImGui::DragFloat("scaleY", &scaleY, 10.0))
        {
            terrainBody->getCollisionShape()->setLocalScaling(btVector3(scaleX, scaleY, scaleZ));
        }
        ImGui::Separator();
        ImGui::Text("Terrain");
        ImGui::Checkbox("wirewrame", &wireframe);
        ImGui::DragInt("level", &level);
        ImGui::DragFloat("scale factor", &scaleFactor, 0.05f);
        ImGui::DragFloat("fogMaxDist", &fogMaxDist, 100.0f);
        ImGui::DragFloat("fogMinDist", &fogMinDist, 100.0f);
        ImGui::ColorEdit3("fogColor", fogColor);
        ImGui::Text("center");
        x = terrainCenter.x;
        y = terrainCenter.y;
        z = terrainCenter.z;
        if (ImGui::DragFloat("terrainCenter-X", &x, 1.0f))
        {
            terrainCenter = glm::vec3(x, y, z);
        }
        if (ImGui::DragFloat("terrainCenter-Z", &z, 1.0))
        {
            terrainCenter = glm::vec3(x, y, z);
        }
        float uvx = uvOffset.x;
        float uvy = uvOffset.y;
        if (ImGui::DragFloat("uvOffset-X", &uvx, 0.001f))
        {
            uvOffset = glm::vec2(uvx, uvy);
        }
        if (ImGui::DragFloat("uvOffset-Y", &uvy, 0.001))
        {
            uvOffset = glm::vec2(uvx, uvy);
        }
        float aox = alphaOffset.x;
        float aoy = alphaOffset.y;
        if (ImGui::DragFloat("alphaOffset-X", &aox, 1.000f))
        {
            alphaOffset = glm::vec2(aox, aoy);
        }
        if (ImGui::DragFloat("alphaOffset-Y", &aoy, 1.000))
        {
            alphaOffset = glm::vec2(aox, aoy);
        }
        ImGui::DragFloat("oneOverWidth", &oneOverWidth, 0.01f);
        ImGui::DragFloat("rotate", &rotate, 0.01f);
    }
    ImGui::End();
}

// https://stackoverflow.com/a/9194117/11601515
// multiple is a power of 2
int roundUp(int numToRound, int multiple)
{
    assert(multiple && ((multiple & (multiple - 1)) == 0));
    return (numToRound + multiple - 1) & -multiple;
}

void createMesh(int m, int n, unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
{
    std::vector<float> vertices;
    std::vector<int> indices;

    for (int i = 0; i < m; i++)
    {
        // vertices
        for (int j = 0; j < n; j++)
        {
            vertices.push_back(i);
            vertices.push_back(j);
        }
        // indices
        if (i == m - 1)
        {
            break;
        }
        int length = n;
        for (int t = 0; t < n - 1; t++)
        {
            int start = n * i + t;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + length);
            indices.push_back(start + 1);
            indices.push_back(start + length);
            indices.push_back(start + length + 1);
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void createOuterCoverMesh(int size, unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
{
    std::vector<float> vertices;
    std::vector<int> indices;
    // --
    for (int i = 0; i < size; i++)
    {
        vertices.push_back(i);
        vertices.push_back(0);
    }
    // --|
    for (int i = 0; i < size; i++)
    {
        vertices.push_back(size);
        vertices.push_back(i);
    }
    // --|
    // --|
    for (int i = size; i > 0; i--)
    {
        vertices.push_back(i);
        vertices.push_back(size);
    }
    // |--|
    // |--|
    for (int i = size; i > 0; i--)
    {
        vertices.push_back(0);
        vertices.push_back(i);
    }
    int half = vertices.size() / 2;
    for (int i = 0; i <= half - 2; i += 2)
    {
        indices.push_back(i);
        indices.push_back(i + 1);
        if (i != half - 2)
        {
            indices.push_back(i + 2);
        }
        else
        {
            indices.push_back(0);
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawAabb);
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

    // Init shader
    Shader normalShader;
    normalShader.init(FileManager::read("assets/shaders/normal-shader.vs"), FileManager::read("assets/shaders/normal-shader.fs"));

    Shader simpleShader;
    simpleShader.init(FileManager::read("assets/shaders/simple-shader.vs"), FileManager::read("assets/shaders/simple-shader.fs"));

    Shader lineShader;
    lineShader.init(FileManager::read("assets/shaders/line-shader.vs"), FileManager::read("assets/shaders/line-shader.fs"));

    Shader terrainShader;
    terrainShader.init(FileManager::read("assets/shaders/terrain-shader.vs"), FileManager::read("assets/shaders/terrain-shader.fs"));

    // Camera
    Camera editorCamera(glm::vec3(0.0f, 240.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 35.0f, -5.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, far);

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

    bool show_overlay = true;
    std::vector<float> listenerOrientation;

    // int n = 255;
    int m = 64; // m = (n+1)/4

    // create mxm
    unsigned int vbo_mxm, vao_mxm, ebo_mxm;
    createMesh(m, m, vbo_mxm, vao_mxm, ebo_mxm);
    // TODO: 1 vao for mx3 and 3mx - rotate
    // create mx3
    unsigned int vbo_mx3, vao_mx3, ebo_mx3;
    createMesh(m, 3, vbo_mx3, vao_mx3, ebo_mx3);
    // create 3xm
    unsigned int vbo_3xm, vao_3xm, ebo_3xm;
    createMesh(3, m, vbo_3xm, vao_3xm, ebo_3xm);
    // TODO: 1 vao for vbo_2m1x2 and vbo_2x2m1 - rotate - mirror
    // create (2m + 1)x2
    unsigned int vbo_2m1x2, vao_2m1x2, ebo_2m1x2;
    createMesh(2 * m + 1, 2, vbo_2m1x2, vao_2m1x2, ebo_2m1x2);
    // create 2x(2m + 1)
    unsigned int vbo_2x2m1, vao_2x2m1, ebo_2x2m1;
    createMesh(2, 2 * m + 1, vbo_2x2m1, vao_2x2m1, ebo_2x2m1);
    // create outer degenerate triangles
    unsigned int vbo_0, vao_0, ebo_0;
    createOuterCoverMesh(4 * (m - 1) + 2, vbo_0, vao_0, ebo_0);

    // 3x3 - finer center
    unsigned int vbo_3x3, vao_3x3, ebo_3x3;
    createMesh(3, 3, vbo_3x3, vao_3x3, ebo_3x3);
    // 2x2 - outside of terrain
    unsigned int vbo_2x2, vao_2x2, ebo_2x2;
    createMesh(2, 2, vbo_2x2, vao_2x2, ebo_2x2);

    // buffer elevationSampler texture
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    float *data = stbi_loadf("assets/images/4096x4096.png", &width, &height, &nrComponents, 1);
    if (data == nullptr)
    {
        fprintf(stderr, "Failed to read heightmap\n");
        return 0;
    }

    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "nrComponents: " << nrComponents << std::endl;

    GLenum format;
    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Terrain Physics
    terrainBody = physicsWorld.createTerrain(
        width,
        height,
		data,
		0,
        1,
        1,
        false
    );

    // TODO: bound to terrain scale factor
    terrainBody->getWorldTransform().setOrigin(btVector3(width/2, scaleFactor/2 + 0.5, height/2));
    terrainBody->getCollisionShape()->setLocalScaling(btVector3(1, scaleFactor, 1));

    // buffer normalMapSampler texture
    unsigned int ntextureID;
    glGenTextures(1, &ntextureID);
    int nwidth, nheight, nnrComponents;
    unsigned char *ndata = stbi_load("assets/images/4096x4096-normal.png", &nwidth, &nheight, &nnrComponents, 0);
    if (ndata == nullptr)
    {
        fprintf(stderr, "Failed to read heightmap\n");
        return 0;
    }

    std::cout << "nwidth: " << nwidth << std::endl;
    std::cout << "nheight: " << nheight << std::endl;
    std::cout << "nnrComponents: " << nnrComponents << std::endl;

    GLenum nformat;
    if (nnrComponents == 1)
        nformat = GL_RED;
    else if (nnrComponents == 3)
        nformat = GL_RGB;
    else if (nnrComponents == 4)
        nformat = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, ntextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, nformat, nwidth, nheight, 0, nformat, GL_UNSIGNED_BYTE, ndata);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(ndata);

    unsigned int ttextureID;
    glGenTextures(1, &ttextureID);
    int twidth, theight, tnrComponents;
    int nrTextures = 6;

    unsigned char *tdata0 = stbi_load("assets/images/water-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata1 = stbi_load("assets/images/sand-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata2 = stbi_load("assets/images/stone-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata3 = stbi_load("assets/images/grass-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata4 = stbi_load("assets/images/rock-1.jpg", &twidth, &theight, &tnrComponents, 0);
    unsigned char *tdata5 = stbi_load("assets/images/snow-1.jpg", &twidth, &theight, &tnrComponents, 0);

    if (tdata0 == nullptr || tdata1 == nullptr || tdata2 == nullptr || tdata3 == nullptr || tdata4 == nullptr || tdata5 == nullptr)
    {
        fprintf(stderr, "Failed to read textures\n");
        return 0;
    }

    std::cout << "twidth: " << twidth << std::endl;
    std::cout << "theight: " << theight << std::endl;
    std::cout << "tnrComponents: " << tnrComponents << std::endl;

    GLenum tformat;
    if (tnrComponents == 1)
        tformat = GL_RED;
    else if (tnrComponents == 3)
        tformat = GL_RGB;
    else if (tnrComponents == 4)
        tformat = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D_ARRAY, ttextureID);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_SRGB8, twidth, theight, nrTextures, 0, tformat, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata0);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata1);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata2);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 3, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata3);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 4, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata4);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 5, twidth, theight, 1, tformat, GL_UNSIGNED_BYTE, tdata5);
    
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    stbi_image_free(tdata0);
    stbi_image_free(tdata1);
    stbi_image_free(tdata2);
    stbi_image_free(tdata3);
    stbi_image_free(tdata4);
    stbi_image_free(tdata5);

    float w = 1.0 / width;
    float h = 1.0 / height;

    std::cout << "w: " << w << std::endl;
    std::cout << "h: " << h << std::endl;

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
        physicsWorld.dynamicsWorld->stepSimulation(deltaTime, 10);

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

        // Update audio listener
        soundEngine.setListenerPosition(editorCamera.position.x, editorCamera.position.y, editorCamera.position.z);
        listenerOrientation.clear();
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
        projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, far);

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

        // Render terrain
        terrainShader.use();
        terrainShader.setFloat("zscaleFactor", scaleFactor);
        terrainShader.setVec3("viewerPos", editorCamera.position);
        terrainShader.setFloat("oneOverWidth", oneOverWidth);
        terrainShader.setVec2("alphaOffset", alphaOffset);
        terrainShader.setVec3("lightDirection", lightPosition);
        terrainShader.setVec2("uvOffset", uvOffset);
        terrainShader.setVec2("terrainSize", glm::vec2(width, height));
        terrainShader.setFloat("fogMaxDist", fogMaxDist);
        terrainShader.setFloat("fogMinDist", fogMinDist);
        terrainShader.setVec4("fogColor", glm::vec4(fogColor[0], fogColor[1], fogColor[2], 1.0f));

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(terrainShader.id, "elevationSampler"), 0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glActiveTexture(GL_TEXTURE0 + 1);
        glUniform1i(glGetUniformLocation(terrainShader.id, "normalMapSampler"), 1);
        glBindTexture(GL_TEXTURE_2D, ntextureID);

        glActiveTexture(GL_TEXTURE0 + 2);
        glUniform1i(glGetUniformLocation(terrainShader.id, "textureSampler"), 2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, ttextureID);

        // for each level
        int lastRoundX, lastRoundZ = 0;
        for (int i = 1; i < level; i++)
        {
            // set param for each footprint
            int scale = pow(2, i - 1);

            int X = -1 * (2 * m * scale) + (int)editorCamera.position.x + (int)terrainCenter.x;
            int Z = -1 * (2 * m * scale) + (int)editorCamera.position.z + (int)terrainCenter.z;

            int x = roundUp(X, scale * 2);
            int z = roundUp(Z, scale * 2);

            // draw each mxm
            glm::mat4 model = editorCamera.getViewMatrix() * glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor)), terrainCenter);
            glm::mat4 mvp = projection * editorCamera.getViewMatrix(); // * model;
            terrainShader.setMat4("worldViewProjMatrix", mvp);

            if (i % 3 == 0)
            {
                terrainShader.setVec3("wireColor", glm::vec3(1, 0.522, 0.522));
            }
            else if (i % 3 == 1)
            {
                terrainShader.setVec3("wireColor", glm::vec3(0.522, 1, 0.682));
            }
            else
            {
                terrainShader.setVec3("wireColor", glm::vec3(0.522, 0.827, 1));
            }

            if (wireframe)
            {
                terrainShader.setBool("wireframe", true);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                terrainShader.setBool("wireframe", false);
            }

            int sizeMM = scale * (m - 1);
            int size2 = scale * 2;
            
            // mxm
            int mmIndices = m * (m - 1) * 6;

            glBindVertexArray(vao_mxm);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, x, z));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * z));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2 + size2, z));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2 + size2), h * z));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * z));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 2 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 2 + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 3 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 3 + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2 + size2, z + sizeMM * 3 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2 + size2), h * (z + sizeMM * 3 + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 3 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 3 + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM * 3 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM * 3 + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM * 2 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM * 2 + size2)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM)));
            glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

            // fine level mxm
            if (i == 1)
            {
                terrainShader.setVec3("wireColor", glm::vec3(1, 1, 1));

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM)));
                glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + 2 * sizeMM + size2, z + sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + 2 * sizeMM + size2), h * (z + sizeMM)));
                glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + 2 * sizeMM + size2));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + 2 * sizeMM + size2)));
                glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + 2 * sizeMM + size2, z + 2 * sizeMM + size2));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + 2 * sizeMM + size2), h * (z + 2 * sizeMM + size2)));
                glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);

                glBindVertexArray(vao_3x3);

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + 2 * sizeMM, z + 2 * sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + 2 * sizeMM), h * (z + 2 * sizeMM)));
                glDrawElements(GL_TRIANGLES, mmIndices, GL_UNSIGNED_INT, 0);
            }
            
            
            // 3xm
            int indices3M = 2 * (m - 1) * 6;

            glBindVertexArray(vao_3xm);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * z));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM * 3 + size2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM * 3 + size2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            // fine level 3xm
            if (i == 1)
            {
                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM)));
                glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2, z + sizeMM * 2 + size2));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2), h * (z + sizeMM * 2 + size2)));
                glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);
            }

            
            // mx3
            glBindVertexArray(vao_mx3);
            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z + sizeMM * 2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * x, h * (z + sizeMM * 2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2, z + sizeMM * 2));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 3 + size2), h * (z + sizeMM * 2)));
            glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

            if (i == 1)
            {
                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 2));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 2)));
                glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);

                terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 2 + size2, z + sizeMM * 2));
                terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM * 2 + size2), h * (z + sizeMM * 2)));
                glDrawElements(GL_TRIANGLES, indices3M, GL_UNSIGNED_INT, 0);
            }
            
            if (i != 1)
            {
                // 2m1x2
                int indices212 = 2 * m * 6;

                glBindVertexArray(vao_2m1x2);

                if (lastRoundZ == z + sizeMM)
                {
                    terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM * 3 + size2 - scale));
                    terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM * 3 + size2 - scale)));
                }
                else
                {
                    terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM));
                    terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM)));
                }
                glDrawElements(GL_TRIANGLES, indices212, GL_UNSIGNED_INT, 0);

                glBindVertexArray(vao_2x2m1);
                if (lastRoundX == x + sizeMM)
                {
                    terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM * 3 + size2 - scale, z + sizeMM));
                    terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (z + sizeMM * 3 + size2 - scale), h * (x + sizeMM)));
                }
                else
                {
                    terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x + sizeMM, z + sizeMM));
                    terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, w * (x + sizeMM), h * (z + sizeMM)));
                }
                glDrawElements(GL_TRIANGLES, indices212, GL_UNSIGNED_INT, 0);
            }

            // outer degenerate triangles
            int indicesOuter = 4 * (m - 1) * 3 * 4;

            terrainShader.setVec3("wireColor", glm::vec3(0, 1, 1));
            glBindVertexArray(vao_0);

            terrainShader.setVec4("scaleFactor", glm::vec4(scale, scale, x, z));
            terrainShader.setVec4("fineTextureBlockOrigin", glm::vec4(w, h, x, z));
            glDrawElements(GL_TRIANGLES, indicesOuter, GL_UNSIGNED_INT, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            lastRoundX = x;
            lastRoundZ = z;
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
        showOverlay(&editorCamera, &soundEngine, &debugDrawer, soundSource, deltaTime, &show_overlay);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Clean Physics
    stbi_image_free(data);

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
