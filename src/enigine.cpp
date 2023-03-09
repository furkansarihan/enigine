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
#include "shadowmap_manager/shadowmap_manager.h"

#include "external/stb_image/stb_image.h"

struct frustum
{
    float near;
    float far;
    float fov;
    float ratio;
    glm::vec3 points[8];
    glm::vec3 lightAABB[8];
};

std::vector<frustum> cascadeFrustums;
int cascadeFrustumSize = 3;
float splitWeight = 0.75f;
float quadScale = 0.2f;
float bias = 0.005;
glm::vec3 terrainBias = glm::vec3(0.020, 0.023, 0.005);
bool drawFrustum = true;
bool drawAABB = false;
bool drawShadowmap = true;
bool showCascade = false;

Camera cascadeCamera(glm::vec3(0, 0, -16), glm::vec3(0, 1, 0), 90, 0, 1, 200);

bool firstMove = true;
float lastX;
float lastY;

// Sphere
glm::vec3 spherePosition = glm::vec3(80.0f, 2.0f, 80.0f);
glm::vec3 groundPos = glm::vec3(0, 0, 0.75);
// Light
glm::vec3 lightPosition = glm::vec3(1.8f, 1.2f, -0.7f);
glm::vec3 lightLookAt = glm::vec3(0, 0, 0);
static float lightColor[3] = {0.1f, 0.1f, 0.1f};
static float ambientColor[3] = {0.4f, 0.4f, 0.4f};
static float specularColor[3] = {0.15f, 0.15f, 0.15f};
static float lightPower = 10.0;
// Camera
static bool followVehicle = false;
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
Shader textureArrayShader;
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
    terrainShadow.init(FileManager::read("../src/assets/shaders/terrain-shadow.vs"), FileManager::read("../src/assets/shaders/depth-shader.fs"));
    lineShader.init(FileManager::read("../src/assets/shaders/line-shader.vs"), FileManager::read("../src/assets/shaders/line-shader.fs"));
    textureShader.init(FileManager::read("../src/assets/shaders/simple-texture.vs"), FileManager::read("../src/assets/shaders/simple-texture.fs"));
    textureArrayShader.init(FileManager::read("../src/assets/shaders/simple-texture.vs"), FileManager::read("../src/assets/shaders/texture-array.fs"));
}

void processCameraInput(GLFWwindow *window, Camera *editorCamera, float deltaTime)
{
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
        ImGui::DragFloat("near", &editorCamera->near, 10.0f);
        ImGui::DragFloat("far", &editorCamera->far, 10.0f);
        ImGui::DragFloat("fov", &editorCamera->fov, 0.01f);
        ImGui::DragFloat("movementSpeed", &editorCamera->movementSpeed, 10.0f);
        ImGui::DragFloat("scaleOrtho", &editorCamera->scaleOrtho, 0.1f);
        ImGui::Checkbox("followVehicle", &followVehicle);
        if (ImGui::RadioButton("perspective", editorCamera->projectionMode == ProjectionMode::Perspective))
        {
            editorCamera->projectionMode = ProjectionMode::Perspective;
        }
        if (ImGui::RadioButton("ortho", editorCamera->projectionMode == ProjectionMode::Ortho))
        {
            editorCamera->projectionMode = ProjectionMode::Ortho;
        }
        ImGui::Separator();
        ImGui::Text("Shadowmap");
        ImGui::Checkbox("drawShadowmap", &drawShadowmap);
        ImGui::Checkbox("drawFrustum", &drawFrustum);
        ImGui::Checkbox("drawAABB", &drawAABB);
        ImGui::Checkbox("showCascade", &showCascade);
        ImGui::DragFloat("quadScale", &quadScale, 0.1f);
        ImGui::DragFloat("splitWeight", &splitWeight, 0.01f);
        ImGui::DragFloat("bias", &bias, 0.001f);
        ImGui::DragFloat("terrainBias0", &terrainBias.x, 0.001f);
        ImGui::DragFloat("terrainBias1", &terrainBias.y, 0.001f);
        ImGui::DragFloat("terrainBias2", &terrainBias.z, 0.001f);
        ImGui::Text("groundPos");
        ImGui::DragFloat("groundPosX", &groundPos.x, 0.5f);
        ImGui::DragFloat("groundPosY", &groundPos.y, 0.5f);
        ImGui::DragFloat("groundPosZ", &groundPos.z, 0.5f);
        ImGui::Text("camPos");
        ImGui::DragFloat("camPosX", &cascadeCamera.position.x, 0.5f);
        ImGui::DragFloat("camPosY", &cascadeCamera.position.y, 0.5f);
        ImGui::DragFloat("camPosZ", &cascadeCamera.position.z, 0.5f);
        // ImGui::Text("camView");
        // ImGui::DragFloat("camViewX", &camView.x, 0.01f);
        // ImGui::DragFloat("camViewY", &camView.y, 0.01f);
        // ImGui::DragFloat("camViewZ", &camView.z, 0.01f);
        ImGui::DragFloat("camNear", &cascadeCamera.near, 1);
        ImGui::DragFloat("camFar", &cascadeCamera.far, 1, 26, 1000);
        ImGui::Separator();
        ImGui::Text("Light");
        ImGui::DragFloat("X", &lightPosition.x, 0.01f);
        ImGui::DragFloat("Y", &lightPosition.y, 0.01f);
        ImGui::DragFloat("Z", &lightPosition.z, 0.01f);
        ImGui::Text("Light - look at");
        ImGui::DragFloat("llaX", &lightLookAt.x, 0.01f);
        ImGui::DragFloat("llaY", &lightLookAt.y, 0.01);
        ImGui::DragFloat("llaZ", &lightLookAt.z, 0.01);
        ImGui::Separator();
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
    float quad_vertices[] = {
        // top left
        -1, 1,
        0.0f, 1.0f,
        // top right
        1, 1,
        1.0f, 1.0f,
        // bottom left
        -1, -1,
        0.0f, 0.0f,
        // bottom right
        1, -1,
        1.0f, 0.0f};
    unsigned int quad_indices[] = {0, 1, 2, 1, 2, 3};
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

// updateSplitDist computes the near and far distances for every frustum slice
// in camera eye space - that is, at what distance does a slice start and end
void updateSplitDist(float nd, float fd)
{
    float lambda = splitWeight;
    float ratio = fd / nd;
    cascadeFrustums.at(0).near = nd;

    for (int i = 1; i < cascadeFrustumSize; i++)
    {
        float si = i / (float)cascadeFrustumSize;

        cascadeFrustums.at(i).near = lambda * (nd * powf(ratio, si)) + (1 - lambda) * (nd + (fd - nd) * si);
        cascadeFrustums.at(i - 1).far = cascadeFrustums.at(i).near * 1.005f;
    }
    cascadeFrustums.at(cascadeFrustumSize - 1).far = fd;
}

// Compute the 8 corner points of the current view frustum
void updateFrustumPoints(frustum &f, glm::vec3 &center, glm::vec3 &view_dir)
{
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 right = normalize(cross(view_dir, up));

    up = normalize(cross(right, view_dir));

    // view_dir must be normalized
    glm::vec3 fc = center + view_dir * f.far;
    glm::vec3 nc = center + view_dir * f.near;

    // TODO: why tan(fov/2) is not represents width?
    float near_height = tan(f.fov / 2.0f) * f.near;
    float near_width = near_height * f.ratio;
    float far_height = tan(f.fov / 2.0f) * f.far;
    float far_width = far_height * f.ratio;

    f.points[0] = nc - up * near_height - right * near_width;
    f.points[1] = nc + up * near_height - right * near_width;
    f.points[2] = nc + up * near_height + right * near_width;
    f.points[3] = nc - up * near_height + right * near_width;

    f.points[4] = fc - up * far_height - right * far_width;
    f.points[5] = fc + up * far_height - right * far_width;
    f.points[6] = fc + up * far_height + right * far_width;
    f.points[7] = fc - up * far_height + right * far_width;
}

// this function builds a projection matrix for rendering from the shadow's POV.
// First, it computes the appropriate z-range and sets an orthogonal projection.
// Then, it translates and scales it, so that it exactly captures the bounding box
// of the current frustum slice
glm::mat4 applyCropMatrix(frustum &f, glm::mat4 lightView)
{
    float maxX, maxY, maxZ, minX, minY, minZ;

    glm::mat4 nv_mvp = lightView;
    glm::vec4 transf;

    // found min-max X, Y, Z
    transf = nv_mvp * glm::vec4(f.points[0], 1.0f);
    minX = transf.x;
    maxX = transf.x;
    minY = transf.y;
    maxY = transf.y;
    minZ = transf.z;
    maxZ = transf.z;
    for (int i = 1; i < 8; i++)
    {
        transf = nv_mvp * glm::vec4(f.points[i], 1.0f);

        if (transf.z > maxZ)
            maxZ = transf.z;
        if (transf.z < minZ)
            minZ = transf.z;

        // eliminate the perspective - for point lights
        transf.x /= transf.w;
        transf.y /= transf.w;

        if (transf.x > maxX)
            maxX = transf.x;
        if (transf.x < minX)
            minX = transf.x;
        if (transf.y > maxY)
            maxY = transf.y;
        if (transf.y < minY)
            minY = transf.y;
    }

    // TODO: extend borders with scene elements

    f.lightAABB[0] = glm::vec3(minX, minY, maxZ);
    f.lightAABB[1] = glm::vec3(maxX, minY, maxZ);
    f.lightAABB[2] = glm::vec3(maxX, maxY, maxZ);
    f.lightAABB[3] = glm::vec3(minX, maxY, maxZ);

    f.lightAABB[4] = glm::vec3(minX, minY, minZ);
    f.lightAABB[5] = glm::vec3(maxX, minY, minZ);
    f.lightAABB[6] = glm::vec3(maxX, maxY, minZ);
    f.lightAABB[7] = glm::vec3(minX, maxY, minZ);

    // TODO: why (-maxZ, -minZ)? direction is negative-z?
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -maxZ, -minZ);

    nv_mvp = projection * nv_mvp;

    float scaleX = 2.0f / (maxX - minX);
    float scaleY = 2.0f / (maxY - minY);
    float offsetX = -0.5f * (maxX + minX) * scaleX;
    float offsetY = -0.5f * (maxY + minY) * scaleY;

    // TODO: convert to these
    // projection = glm::scale(projection, glm::vec3(scaleX, scaleY, 1));
    // projection = glm::translate(projection, glm::vec3(offsetX, offsetY, 1));

    glm::mat4 crop = glm::mat4(1.0f);
    crop[0][0] = scaleX;
    crop[1][1] = scaleY;
    crop[0][3] = offsetX;
    crop[1][3] = offsetY;
    // TODO: why transpose?
    crop = glm::transpose(crop);

    projection = projection * crop;

    return projection;
}

void buildCascadeFrustums(Camera *camera, float screenWidth, float screenHeight)
{
    cascadeFrustums.clear();

    for (int i = 0; i < cascadeFrustumSize; i++)
    {
        // note that fov is in radians here and in OpenGL it is in degrees.
        // the 0.2f factor is important because we might get artifacts at
        // the screen borders.
        frustum frustum;
        frustum.fov = camera->fov + 0.2f;
        frustum.ratio = (double)screenWidth / (double)screenHeight;

        cascadeFrustums.push_back(frustum);
    }

    updateSplitDist(camera->near, camera->far);

    for (int i = 0; i < cascadeFrustumSize; i++)
    {
        updateFrustumPoints(cascadeFrustums.at(i), camera->position, camera->front);
    }
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
    Model suzanne("assets/models/suzanne.obj");

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
    ShadowmapManager shadowmapManager(cascadeFrustumSize, 1024);

    // Shadowmap lookup matrices
    unsigned int ubo;
    glGenBuffers(1, &ubo);

    GLuint uniformBlockIndex1 = glGetUniformBlockIndex(simpleShadow.id, "matrices");
    glUniformBlockBinding(simpleShadow.id, uniformBlockIndex1, 0);

    GLuint uniformBlockIndex2 = glGetUniformBlockIndex(terrainShader.id, "matrices");
    glUniformBlockBinding(terrainShader.id, uniformBlockIndex2, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * cascadeFrustumSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Shadowmap display quad
    unsigned int q_vbo, q_vao, q_ebo;
    createQuad(q_vbo, q_vao, q_ebo);

    // camera frustum
    unsigned int c_vbo, c_vao, c_ebo;
    glGenVertexArrays(1, &c_vao);
    glGenBuffers(1, &c_vbo);
    glGenBuffers(1, &c_ebo);

    // light view frustum
    unsigned int c_vbo_2, c_vao_2, c_ebo_2;
    glGenVertexArrays(1, &c_vao_2);
    glGenBuffers(1, &c_vbo_2);
    glGenBuffers(1, &c_ebo_2);

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
        glm::mat4 projection = editorCamera.getProjectionMatrix((float)screenWidth, (float)screenHeight);

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

        // Shadowmap

        // Build cascade frustums
        buildCascadeFrustums(&cascadeCamera, screenWidth, screenHeight);

        glm::mat4 depthVpMatrices[cascadeFrustumSize];
        glm::mat4 depthPMatrices[cascadeFrustumSize];
        glm::mat4 depthViewMatrix = glm::lookAt(lightPosition, lightLookAt, glm::vec3(0, 1, 0));

        for (int i = 0; i < cascadeFrustumSize; i++)
        {
            depthPMatrices[i] = applyCropMatrix(cascadeFrustums.at(i), depthViewMatrix);
            depthVpMatrices[i] = depthPMatrices[i] * depthViewMatrix;
        }

        shadowmapManager.bindFramebuffer();
        for (int i = 0; i < cascadeFrustumSize; i++)
        {
            shadowmapManager.bindTextureArray(i);
            glm::mat4 depthVP = depthVpMatrices[i];
            // We don't use bias in the shader, but instead we draw back faces,
            // which are already separated from the front faces by a small distance
            // (if your geometry is made this way)
            // glCullFace(GL_FRONT);

            // Draw terrain
            terrain.drawDepth(terrainShadow, editorCamera.position, depthViewMatrix, depthPMatrices[i]);

            // Draw objects
            glm::vec4 objectColor(0.6f, 0.6f, 0.6f, 1.0f);

            glm::vec3 position = glm::vec3(0, 2, 0);
            glm::vec3 scale = glm::vec3(4, 4, 4);
            glm::mat4 objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.use();
            depthShader.setBool("DrawDepth", false);
            depthShader.setMat4("MVP", depthVP * objectModel);
            depthShader.setVec4("DiffuseColor", objectColor);
            suzanne.draw(depthShader);

            // wall
            position = glm::vec3(0, 1.25, -9);
            scale = glm::vec3(10.0f, 4.0f, 1.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.use();
            depthShader.setBool("DrawDepth", false);
            depthShader.setMat4("MVP", depthVP * objectModel);
            depthShader.setVec4("DiffuseColor", objectColor);
            cube.draw(depthShader);

            // ground
            position = groundPos;
            scale = glm::vec3(20.0f, 2.0f, 100.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            depthShader.use();
            depthShader.setBool("DrawDepth", false);
            depthShader.setMat4("MVP", depthVP * objectModel);
            depthShader.setVec4("DiffuseColor", objectColor);
            cube.draw(depthShader);

            // ball
            for (int i = 2; i < 30; i++)
            {
                position = glm::vec3(0, 2, 3 * i);
                scale = glm::vec3(2.0f, 2.0f, 2.0f);
                objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
                depthShader.use();
                depthShader.setBool("DrawDepth", false);
                depthShader.setMat4("MVP", depthVP * objectModel);
                depthShader.setVec4("DiffuseColor", objectColor);
                sphere.draw(depthShader);
            }

            // ball
            for (int i = 2; i < 30; i++)
            {
                position = glm::vec3(-20, 2, 3 * i);
                scale = glm::vec3(2.0f, 2.0f, 2.0f);
                objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
                depthShader.use();
                depthShader.setBool("DrawDepth", false);
                depthShader.setMat4("MVP", depthVP * objectModel);
                depthShader.setVec4("DiffuseColor", objectColor);
                sphere.draw(depthShader);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glCullFace(GL_BACK);
        glViewport(0, 0, screenWidth, screenHeight);

        // Draw shadowmap
        if (drawShadowmap)
        {
            textureArrayShader.use();

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(textureArrayShader.id, "renderedTexture"), 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, shadowmapManager.m_textureArray);

            for (int i = 0; i < cascadeFrustumSize; i++)
            {
                if (!drawShadowmap)
                    break;
                glm::mat4 model = glm::mat4(1.0f);

                glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);    // Camera position
                glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f); // Point camera is looking at
                glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     // Up direction

                glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

                float aspectRatio = (float)screenWidth / screenHeight;
                glm::vec3 scale = glm::vec3(quadScale / aspectRatio, quadScale, 1.0f);

                float posX = 1.0f - scale.x * ((i + 0.5) * 2);
                float posY = -1.0f + scale.y;
                glm::vec3 position = glm::vec3(posX, posY, 0);

                glm::mat4 projection = glm::ortho(-1, 1, -1, 1, 0, 2);
                projection = glm::scale(glm::translate(projection, position), scale);

                textureArrayShader.setMat4("MVP", projection * view * model);
                textureArrayShader.setInt("layer", i);

                glBindVertexArray(q_vao);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }
        }

        // Render scene
        glm::mat4 biasMatrix(
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0);

        // setup shadowmap lookup matrices
        glm::mat4 depthBiasVPMatrices[cascadeFrustumSize];

        glm::mat4 camViewMatrix = glm::lookAt(cascadeCamera.position, cascadeCamera.front, glm::vec3(0, 1, 0));
        glm::vec4 frustumDistances;

        for (int i = 0; i < cascadeFrustumSize; i++)
        {
            depthBiasVPMatrices[i] = biasMatrix * depthVpMatrices[i];
            frustumDistances[i] = cascadeFrustums[i].far;
        }

        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * cascadeFrustumSize, depthBiasVPMatrices, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

        terrain.drawColor(terrainShader, editorCamera.position, lightPosition, glm::vec3(lightColor[0], lightColor[1], lightColor[2]),
                          lightPower, editorCamera.getViewMatrix(), projection, shadowmapManager.m_textureArray,
                          // editorCamera.position, editorCamera.front,
                          cascadeCamera.position, cascadeCamera.front,
                          frustumDistances, showCascade, terrainBias);

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
            simpleShadow.setVec3("CamPos", cascadeCamera.position);
            simpleShadow.setVec3("CamView", cascadeCamera.front);
            // simpleShadow.setVec3("CamPos", editorCamera.position);
            // simpleShadow.setVec3("CamView", editorCamera.front);
            simpleShadow.setVec4("FrustumDistances", frustumDistances);
            simpleShadow.setBool("ShowCascade", showCascade);

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
            simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
            suzanne.draw(simpleShadow);

            // wall
            position = glm::vec3(0, 1.25, -9);
            scale = glm::vec3(10.0f, 4.0f, 1.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
            cube.draw(simpleShadow);

            // ground
            position = groundPos;
            scale = glm::vec3(20.0f, 2.0f, 100.0f);
            objectModel = glm::translate(glm::scale(glm::mat4(1.0f), scale), position);
            simpleShadow.setMat4("MVP", projection * editorCamera.getViewMatrix() * objectModel);
            simpleShadow.setMat4("V", editorCamera.getViewMatrix());
            simpleShadow.setMat4("M", objectModel);
            simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
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
                simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
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
                simpleShadow.setVec3("LightInvDirection_worldspace", lightPosition);
                sphere.draw(simpleShadow);
            }
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

        // Draw frustum
        for (int i = 0; drawFrustum && i < cascadeFrustumSize; i++)
        {
            GLuint indices[] = {
                // Near plane
                0, 1, 1, 2, 2, 3, 3, 0,
                // Far plane
                4, 5, 5, 6, 6, 7, 7, 4,
                // Connections between planes
                0, 4, 1, 5, 2, 6, 3, 7};

            glBindVertexArray(c_vao);

            glBindBuffer(GL_ARRAY_BUFFER, c_vbo);
            glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), cascadeFrustums.at(i).points, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(float), indices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

            glm::mat4 mvp = projection * editorCamera.getViewMatrix();
            simpleShader.use();
            simpleShader.setMat4("MVP", mvp);
            simpleShader.setVec4("DiffuseColor", glm::vec4(0.0, 1.0, 1.0, 1.0f));

            glBindVertexArray(c_vao);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Draw light AABB
        for (int i = 0; drawAABB && i < cascadeFrustumSize; i++)
        {
            // Define indices
            GLuint indices[] = {
                0, 1, 2, 2, 3, 0, // Front face
                1, 5, 6, 6, 2, 1, // Right face
                5, 4, 7, 7, 6, 5, // Back face
                4, 0, 3, 3, 7, 4, // Left face
                3, 2, 6, 6, 7, 3, // Top face
                0, 1, 5, 5, 4, 0, // Bottom face
            };

            glBindVertexArray(c_vao_2);

            glBindBuffer(GL_ARRAY_BUFFER, c_vbo_2);
            glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), cascadeFrustums.at(i).lightAABB, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ebo_2);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(float), indices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

            glm::mat4 inverseViewMatrix = glm::inverse(depthViewMatrix);
            glm::mat4 mvp = projection * editorCamera.getViewMatrix() * inverseViewMatrix;
            simpleShader.use();
            simpleShader.setMat4("MVP", mvp);
            glm::vec4 color = glm::vec4(1.0, 1.0, 1.0, 0.2f);
            color[i] *= 0.7;
            simpleShader.setVec4("DiffuseColor", color);

            glBindVertexArray(c_vao_2);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            simpleShader.setVec4("DiffuseColor", glm::vec4(0.0, 0.0, 0.0, 1.0f));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindVertexArray(0);
        }

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
