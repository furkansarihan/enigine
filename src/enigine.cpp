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
#include "shadow_manager/shadow_manager.h"
#include "post_process/post_process.h"
#include "pbr_manager/pbr_manager.h"
#include "animation/animator.h"
#include "character_controller/character_controller.h"

#include "external/stb_image/stb_image.h"

// Shadowmap
float quadScale = 0.2f;
float bias = 0.005;
bool drawFrustum = false;
bool drawAABB = false;
bool drawShadowmap = false;
// Sphere
glm::vec3 modelPosition = glm::vec3(200.0f, 10.5f, 200.0f);
glm::vec3 modelRotate = glm::vec3(0.0f, 0.0f, 0.0f);
float modelScale = 2.0;
int selectedAnimPose = 0;
glm::vec3 groundPos = glm::vec3(0, 0, 0.75);
// Light
static float lightColor[3] = {0.1f, 0.1f, 0.1f};
static float ambientColor[3] = {0.4f, 0.4f, 0.4f};
static float specularColor[3] = {0.15f, 0.15f, 0.15f};
static float lightPower = 10.0;
static float radius = 12.0;
static float speed = 2.0;
// PBR
static float albedo[3] = {0.5f, 0.0f, 0.0f};
static float ao = 1.0;
glm::vec3 lightPositions[] = {
    glm::vec3(0.0f, 0.0f, 10.0f),
};
glm::vec3 lightColors[] = {
    glm::vec3(350.0f, 410.0f, 458.0f),
};
// Camera
bool firstPerson = false;
bool followVehicle = false;
bool followCharacter = true;
float followDistance = 10.0f;
glm::vec3 followOffset = glm::vec3(0.0f, 2.0f, 0.0f);
float blurOffset = 0.01;
bool firstMove = true;
float lastX;
float lastY;
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
Shader postProcessShader;
Shader hdrToCubemapShader;
Shader cubemapShader;
Shader irradianceShader;
Shader pbrShader;
Shader prefilterShader;
Shader brdfShader;
Shader grassShader;
Shader stoneShader;
Shader animShader;
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
    postProcessShader.init(FileManager::read("../src/assets/shaders/post-process.vs"), FileManager::read("../src/assets/shaders/post-process.fs"));
    hdrToCubemapShader.init(FileManager::read("../src/assets/shaders/hdr-to-cubemap.vs"), FileManager::read("../src/assets/shaders/hdr-to-cubemap.fs"));
    cubemapShader.init(FileManager::read("../src/assets/shaders/cubemap.vs"), FileManager::read("../src/assets/shaders/cubemap.fs"));
    irradianceShader.init(FileManager::read("../src/assets/shaders/cubemap.vs"), FileManager::read("../src/assets/shaders/irradiance.fs"));
    pbrShader.init(FileManager::read("../src/assets/shaders/pbr.vs"), FileManager::read("../src/assets/shaders/pbr.fs"));
    prefilterShader.init(FileManager::read("../src/assets/shaders/cubemap.vs"), FileManager::read("../src/assets/shaders/prefilter.fs"));
    brdfShader.init(FileManager::read("../src/assets/shaders/post-process.vs"), FileManager::read("../src/assets/shaders/brdf.fs"));
    grassShader.init(FileManager::read("../src/assets/shaders/grass.vs"), FileManager::read("../src/assets/shaders/grass.fs"));
    stoneShader.init(FileManager::read("../src/assets/shaders/stone.vs"), FileManager::read("../src/assets/shaders/stone.fs"));
    animShader.init(FileManager::read("../src/assets/shaders/anim.vs"), FileManager::read("../src/assets/shaders/anim.fs"));
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

static void showOverlay(CharacterController *characterController, Animator *animator, btRigidBody *characterBody, PostProcess *postProcess, ShadowManager *shadowManager, Camera *editorCamera, Vehicle *vehicle, SoundEngine *soundEngine, Terrain *terrain, DebugDrawer *debugDrawer, SoundSource soundSource, float deltaTime, bool *p_open)
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
        ImGui::DragFloat("near", &editorCamera->near, 0.001f);
        ImGui::DragFloat("far", &editorCamera->far, 10.0f);
        ImGui::DragFloat("fov", &editorCamera->fov, 0.01f);
        ImGui::DragFloat("movementSpeed", &editorCamera->movementSpeed, 10.0f);
        ImGui::DragFloat("scaleOrtho", &editorCamera->scaleOrtho, 0.1f);
        // ImGui::DragFloat("blurOffset", &blurOffset, 0.001f);
        ImGui::DragFloat("followDistance", &followDistance, 0.1f);
        ImGui::DragFloat("followOffsetY", &followOffset.y, 0.1f);
        ImGui::Checkbox("followVehicle", &followVehicle);
        ImGui::Checkbox("followCharacter", &followCharacter);
        // TODO: change camera min pitch
        if (ImGui::Checkbox("firstPerson", &firstPerson))
        {
            if (firstPerson)
            {
                followDistance = -1.0f;
                followOffset.y = 3.3f;
            }
            else
            {
                followDistance = 10.0f;
                followOffset.y = 1.5f;
            }
        }
        if (ImGui::RadioButton("perspective", editorCamera->projectionMode == ProjectionMode::Perspective))
        {
            editorCamera->projectionMode = ProjectionMode::Perspective;
        }
        if (ImGui::RadioButton("ortho", editorCamera->projectionMode == ProjectionMode::Ortho))
        {
            editorCamera->projectionMode = ProjectionMode::Ortho;
        }
        ImGui::Separator();
        ImGui::Text("Animation");
        int max = animator->m_animations.size() - 1;
        ImGui::DragInt("fromIndex", &animator->m_state.fromIndex, 1, 0, max);
        ImGui::DragInt("toIndex", &animator->m_state.toIndex, 1, 0, max);
        ImGui::DragFloat("blendFactor", &animator->m_state.blendFactor, 0.01f, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::DragInt("selectedAnimPose", &selectedAnimPose, 1, 0, animator->m_state.poses.size() - 1);
        if (selectedAnimPose >= 0 && selectedAnimPose < animator->m_state.poses.size())
        {
            AnimPose *animPose = &animator->m_state.poses[selectedAnimPose];
            ImGui::Text("animPose.index: %d", animPose->index);
            ImGui::DragFloat("animPose.blendFactor", &animPose->blendFactor, 0.01f, 0.0f, 1.0f);
            auto bonesMap = &animator->m_animations[animPose->index]->m_bones;
            for (auto it = bonesMap->begin(); it != bonesMap->end(); ++it)
            {
                Bone *bone = it->second;
                ImGui::DragFloat(bone->m_Name.c_str(), &bone->m_blendFactor, 0.01f, 0.0f, 1.0f);
            }
        }
        ImGui::Separator();
        // ImGui::Text("left plane: (%.3f, %.3f, %.3f, %.3f)", terrain->m_planes[0].x, terrain->m_planes[0].y, terrain->m_planes[0].z, terrain->m_planes[0].w);
        // ImGui::Text("right plane: (%.3f, %.3f, %.3f, %.3f)", terrain->m_planes[1].x, terrain->m_planes[1].y, terrain->m_planes[1].z, terrain->m_planes[1].w);
        ImGui::Text("camPos");
        ImGui::DragFloat("camPosX", &shadowManager->m_camera->position.x, 0.5f);
        ImGui::DragFloat("camPosY", &shadowManager->m_camera->position.y, 0.5f);
        ImGui::DragFloat("camPosZ", &shadowManager->m_camera->position.z, 0.5f);
        ImGui::Text("camView");
        ImGui::DragFloat("camViewX", &shadowManager->m_camera->front.x, 0.01f);
        ImGui::DragFloat("camViewY", &shadowManager->m_camera->front.y, 0.01f);
        ImGui::DragFloat("camViewZ", &shadowManager->m_camera->front.z, 0.01f);
        shadowManager->m_camera->front = glm::normalize(shadowManager->m_camera->front);
        ImGui::DragFloat("camNear", &shadowManager->m_near, 1);
        ImGui::DragFloat("camFar", &shadowManager->m_far, 1, 26, 1000);
        if (ImGui::Button("refresh shaders"))
        {
            initShaders();
        }
        ImGui::Text("model");
        ImGui::DragFloat("modelX", &modelPosition.x, 0.5f);
        ImGui::DragFloat("modelY", &modelPosition.y, 0.5f);
        ImGui::DragFloat("modelZ", &modelPosition.z, 0.5f);
        ImGui::DragFloat("modelScale", &modelScale, 0.01f);
        ImGui::DragFloat("modelRotateX", &modelRotate.x, 0.01f);
        ImGui::DragFloat("modelRotateY", &modelRotate.y, 0.01f);
        ImGui::DragFloat("modelRotateZ", &modelRotate.z, 0.01f);
        ImGui::Separator();
        ImGui::Text("Shadowmap");
        ImGui::Checkbox("drawShadowmap", &drawShadowmap);
        ImGui::Checkbox("drawFrustum", &drawFrustum);
        ImGui::Checkbox("drawAABB", &drawAABB);
        ImGui::Checkbox("showCascade", &terrain->showCascade);
        ImGui::DragFloat("quadScale", &quadScale, 0.1f);
        ImGui::DragFloat("splitWeight", &shadowManager->m_splitWeight, 0.01f);
        ImGui::DragFloat("bias", &bias, 0.001f);
        ImGui::DragFloat("terrainBias0", &terrain->shadowBias.x, 0.001f);
        ImGui::DragFloat("terrainBias1", &terrain->shadowBias.y, 0.001f);
        ImGui::DragFloat("terrainBias2", &terrain->shadowBias.z, 0.001f);
        ImGui::Separator();
        ImGui::Text("Light");
        ImGui::DragFloat("X", &shadowManager->m_lightPos.x, 0.01f);
        ImGui::DragFloat("Y", &shadowManager->m_lightPos.y, 0.01f);
        ImGui::DragFloat("Z", &shadowManager->m_lightPos.z, 0.01f);
        ImGui::Text("Light - look at");
        ImGui::DragFloat("llaX", &shadowManager->m_lightLookAt.x, 0.01f);
        ImGui::DragFloat("llaY", &shadowManager->m_lightLookAt.y, 0.01);
        ImGui::DragFloat("llaZ", &shadowManager->m_lightLookAt.z, 0.01);
        ImGui::Separator();
        ImGui::DragFloat("power", &lightPower, 0.1);
        ImGui::DragFloat("radius", &radius, 0.1);
        ImGui::DragFloat("speed", &speed, 0.01);
        ImGui::ColorEdit3("lightColor", lightColor);
        ImGui::ColorEdit3("ambientColor", ambientColor);
        ImGui::ColorEdit3("specularColor", specularColor);
        ImGui::Separator();
        ImGui::Text("Post process");
        ImGui::DragFloat("exposure", &postProcess->m_exposure, 0.001);
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
            debugDrawer->setDebugMode(debugEnabled ? btIDebugDraw::DBG_DrawWireframe : btIDebugDraw::DBG_NoDebug);
        }
        int lines = debugDrawer->getLines().size();
        ImGui::DragInt("lines", &lines);
        ImGui::Separator();
        ImGui::Text("Character");
        ImGui::Text("m_moveDir: (%.1f, %.1f)", characterController->m_moveDir.x, characterController->m_moveDir.z);
        ImGui::Text("m_moving: %d", characterController->m_moving);
        ImGui::Text("m_jumping: %d", characterController->m_jumping);
        ImGui::Text("m_falling: %d", characterController->m_falling);
        ImGui::Text("m_onGround: %d", characterController->m_onGround);
        ImGui::Text("m_running: %d", characterController->m_running);
        btVector3 linearVelocity = characterBody->getLinearVelocity();
        btVector3 angularVelocity = characterBody->getAngularVelocity();
        ImGui::Text("linearVelocity: (%.1f, %.1f, %.1f)", linearVelocity.getX(), linearVelocity.getY(), linearVelocity.getZ());
        ImGui::Text("angularVelocity: (%.1f, %.1f, %.1f)", angularVelocity.getX(), angularVelocity.getY(), angularVelocity.getZ());
        ImGui::Text("linearSpeed: %.1f", linearVelocity.distance(btVector3(0, 0, 0)));
        ImGui::Text("hasContactResponse: %d", characterBody->hasContactResponse());
        ImGui::Text("m_elevationDistance: %.3f", characterController->m_elevationDistance);
        ImGui::Text("m_speedAtJumpStart: %.3f", characterController->m_speedAtJumpStart);
        ImGui::DragFloat("m_moveForce", &characterController->m_moveForce, 0.1f, 0);
        ImGui::DragFloat("m_jumpForce", &characterController->m_jumpForce, 0.1f, 0);
        ImGui::DragFloat("m_maxWalkSpeed", &characterController->m_maxWalkSpeed, 0.1f, 0);
        ImGui::DragFloat("m_maxRunSpeed", &characterController->m_maxRunSpeed, 0.1f, 0);
        ImGui::DragFloat("m_toIdleForce", &characterController->m_toIdleForce, 0.1f, 0);
        ImGui::DragFloat("m_toIdleForceHoriz", &characterController->m_toIdleForceHoriz, 0.1f, 0);
        ImGui::DragFloat("m_groundTreshold", &characterController->m_groundTreshold, 0.1f, 0);
        float gravityY = characterBody->getGravity().getY();
        if (ImGui::DragFloat("gravity", &gravityY, 0.1f))
        {
            characterBody->setGravity(btVector3(0, gravityY, 0));
        }
        float sX = characterBody->getWorldTransform().getOrigin().getX();
        float sY = characterBody->getWorldTransform().getOrigin().getY();
        float sZ = characterBody->getWorldTransform().getOrigin().getZ();
        if (ImGui::DragFloat("sX", &sX, 0.1f))
        {
            characterBody->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
            characterBody->setLinearVelocity(btVector3(0, 0, 0));
        }
        if (ImGui::DragFloat("sY", &sY, 0.1))
        {
            characterBody->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
            characterBody->setLinearVelocity(btVector3(0, 0, 0));
        }
        if (ImGui::DragFloat("sZ", &sZ, 0.1))
        {
            characterBody->getWorldTransform().setOrigin(btVector3(sX, sY, sZ));
            characterBody->setLinearVelocity(btVector3(0, 0, 0));
        }
        float characterMass = characterBody->getMass();
        if (ImGui::DragFloat("characterMass", &characterMass, 0.1))
        {
            btVector3 interia;
            characterBody->getCollisionShape()->calculateLocalInertia(characterMass, interia);
            characterBody->setMassProps(characterMass, interia);
        }
        float friction = characterBody->getFriction();
        if (ImGui::DragFloat("friction", &friction, 0.1))
        {
            characterBody->setFriction(friction);
        }
        float linearDamping = characterBody->getLinearDamping();
        float angularDamping = characterBody->getLinearDamping();
        if (ImGui::DragFloat("linearDamping", &linearDamping, 0.1))
        {
            characterBody->setDamping(linearDamping, angularDamping);
        }
        if (ImGui::DragFloat("angularDamping", &angularDamping, 0.1))
        {
            characterBody->setDamping(linearDamping, angularDamping);
        }
        ImGui::Separator();
        ImGui::Text("Terrain");
        ImGui::Checkbox("wirewrame", &terrain->wireframe);
        ImGui::DragInt("level", &terrain->level);
        if (ImGui::DragFloat("scaleHoriz", &terrain->m_scaleHoriz, 0.05f))
        {
            terrain->updateHorizontalScale();
        }
        if (ImGui::DragFloat("minHeight", &terrain->m_minHeight, 1.0f))
        {
            terrain->updateHorizontalScale();
        }
        if (ImGui::DragFloat("maxHeight", &terrain->m_maxHeight, 1.0f))
        {
            terrain->updateHorizontalScale();
        }
        ImGui::DragFloat("fogMaxDist", &terrain->fogMaxDist, 100.0f);
        ImGui::DragFloat("fogMinDist", &terrain->fogMinDist, 100.0f);
        ImGui::ColorEdit4("fogColor", &terrain->fogColor[0]);
        ImGui::DragFloat("ambientMult", &terrain->ambientMult, 0.01f);
        ImGui::DragFloat("diffuseMult", &terrain->diffuseMult, 0.01f);
        ImGui::DragFloat("specularMult", &terrain->specularMult, 0.01f);
        ImGui::DragFloat("terrainCenter-X", &terrain->terrainCenter.x, 1.0f);
        ImGui::DragFloat("terrainCenter-Z", &terrain->terrainCenter.z, 1.0);
        ImGui::DragFloat("uvOffset-X", &terrain->uvOffset.x, 0.001f);
        ImGui::DragFloat("uvOffset-Y", &terrain->uvOffset.y, 0.001);
        ImGui::DragInt("grassTileSize", &terrain->m_grassTileSize, 1, 0, 128);
        ImGui::DragFloat("grassDensity", &terrain->m_grassDensity, 0.01, 0, 10);
        ImGui::DragInt("stoneTileSize", &terrain->m_stoneTileSize, 1, 0, 128);
        ImGui::DragFloat("stoneDensity", &terrain->m_stoneDensity, 0.01, 0, 10);
        ImGui::DragFloat("windIntensity", &terrain->m_windIntensity, 0.2, 0, 50);
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
    // glDepthFunc(GL_LESS);
    glDepthFunc(GL_LEQUAL);
    // Trasparency
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

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
        soundSource = soundEngine.loadSource("../src/assets/sounds/rain-loop-1648m.wav");
    }
    catch (const char *e)
    {
        std::cerr << e << std::endl;
        return 0;
    }

    soundEngine.setSourcePosition(soundSource, modelPosition.x, modelPosition.y, modelPosition.z);
    // soundEngine.playSource(soundSource);

    // Init Physics
    PhysicsWorld physicsWorld;

    DebugDrawer debugDrawer;
    debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    physicsWorld.dynamicsWorld->setDebugDrawer(&debugDrawer);

    btRigidBody *groundBody = physicsWorld.createBox(0, btVector3(10.0f, 10.0f, 10.0f), btVector3(0, -10, 0));
    // set ground bouncy
    groundBody->setRestitution(0.5f);

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
    // Model spherePBR("../src/assets/spaceship/sphere.obj");
    Model grass("../src/assets/terrain/grass.obj");
    Model stone("../src/assets/terrain/stone.obj");

    // Animation
    // TODO: single aiScene read
    Model animModel("../src/assets/gltf/character.glb");
    Animation animation0("../src/assets/gltf/character.glb", "idle", &animModel);
    Animation animation1("../src/assets/gltf/character.glb", "walking", &animModel);
    Animation animation2("../src/assets/gltf/character.glb", "left", &animModel);
    Animation animation3("../src/assets/gltf/character.glb", "right", &animModel);
    std::vector<Animation *> animations;
    animations.push_back(&animation0);
    animations.push_back(&animation1);
    animations.push_back(&animation2);
    animations.push_back(&animation3);
    Animator animator(animations);

    // idle - walk
    animator.m_state.fromIndex = 0;
    animator.m_state.toIndex = 1;
    animator.m_state.blendFactor = 0.0f;

    // set blend mask for turn-right and turn-left
    std::unordered_map<std::string, float> blendMask;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;

    animation2.setBlendMask(blendMask);
    animation3.setBlendMask(blendMask);

    // turn-left pose
    AnimPose animPose;
    animPose.index = 2;
    animPose.blendFactor = 0.0f;
    animator.m_state.poses.push_back(animPose);
    // turn-right pose
    animPose.index = 3;
    animator.m_state.poses.push_back(animPose);

    // Init shaders
    initShaders();

    // Camera
    Camera editorCamera(modelPosition + glm::vec3(10.0f, 17.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -124.0f, -10.0f);

    // Character
    // TODO: bound to character controller
    btRigidBody *characterBody = physicsWorld.createCapsule(10.0f, 1.0f, 0.5f, 2.0f, btVector3(modelPosition.x, modelPosition.y, modelPosition.z));
    characterBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    characterBody->setDamping(0.9f, 0.9f);
    characterBody->setFriction(0.0f);
    characterBody->setGravity(btVector3(0, -20.0f, 0));

    CharacterController characterController(physicsWorld.dynamicsWorld, characterBody, &editorCamera);

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

    // Vehicle
    Vehicle vehicle(&physicsWorld, btVector3(modelPosition.x + 40, 5, modelPosition.z + 40));
    glfwSetWindowUserPointer(window, &vehicle);
    glfwSetKeyCallback(window, vehicle.staticKeyCallback);

    // Shadowmap
    std::vector<unsigned int> shaderIds;
    shaderIds.push_back(simpleShadow.id);
    shaderIds.push_back(terrainShadow.id);

    ShadowManager shadowManager(shaderIds);
    shadowManager.m_camera = &editorCamera;
    ShadowmapManager shadowmapManager(shadowManager.m_splitCount, 1024);

    // Terrain
    // Terrain terrain(&physicsWorld, "../src/assets/images/4096x4096.png", 0.0f, 798.0f, 2.0f);
    // Terrain terrain(&physicsWorld, "../src/assets/images/height-1.png", -1.0f, 517.0f, 6.0f);
    // Terrain terrain(&physicsWorld, "../src/assets/images/height-2.png", 0.0f, 428.0f, 8.0f);
    // Terrain terrain(&physicsWorld, "../src/assets/images/height-3.png", 0.0f, 105.0f, 1.0f);
    // Terrain terrain(&physicsWorld, "../src/assets/images/height-4.png", 0.0f, 508.0f, 1.0f);
    Terrain terrain(&physicsWorld, "../src/assets/images/test-5.png", -1.0f, 517.0f, 6.0f);

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

    // Post process
    PostProcess postProcess((float)screenWidth, (float)screenHeight);

    // PBR
    PbrManager pbrManager;
    pbrManager.setupCubemap(cube, hdrToCubemapShader);
    // pbrManager.setupIrradianceMap(cube, irradianceShader);
    // pbrManager.setupPrefilterMap(cube, prefilterShader);
    // pbrManager.setupBrdfLUTTexture(q_vao, brdfShader);

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

        // update animation
        animator.update(deltaTime);

        // Update Physics
        physicsWorld.dynamicsWorld->stepSimulation(deltaTime, 1);
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
        if (characterBody && characterBody->getMotionState())
        {
            btTransform trans;
            characterBody->getMotionState()->getWorldTransform(trans);
            modelPosition = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            modelPosition -= glm::vec3(0, characterController.m_halfHeight, 0);
            float cos;
            float angle;
            if (characterController.m_moveDir.x < 0)
            {
                cos = -characterController.m_moveDir.z;
                angle = glm::acos(cos) + M_PI;
            }
            else
            {
                cos = characterController.m_moveDir.z;
                angle = glm::acos(cos);
            }
            modelRotate.y = angle;
            soundEngine.setSourcePosition(soundSource, modelPosition.x, modelPosition.y, modelPosition.z);
            float clamped = std::max(0.0f, std::min(characterController.m_verticalSpeed / characterController.m_maxWalkSpeed, 1.0f));
            animator.m_state.blendFactor = clamped;
        }

        // Vehicle
        if (followVehicle && vehicle.m_carChassis && vehicle.m_carChassis->getMotionState())
        {
            vehicle.update(window, deltaTime);
            btTransform trans;
            vehicle.m_carChassis->getMotionState()->getWorldTransform(trans);
            glm::vec3 vehiclePosition = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            editorCamera.position = vehiclePosition - editorCamera.front * glm::vec3(followDistance) + followOffset;
        }

        // Character
        if (followCharacter)
        {
            characterController.update(window, deltaTime);
            editorCamera.position = modelPosition - editorCamera.front * glm::vec3(followDistance) + followOffset;
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

        // Render to post process texture
        postProcess.updateFramebuffer((float)screenWidth, (float)screenHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, postProcess.m_framebufferObject);
        glViewport(0, 0, screenWidth, screenHeight);
        glClearColor(0.46f, 0.71f, 0.98f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        // animation
        {
            animShader.use();
            animShader.setMat4("projection", projection);
            animShader.setMat4("view", editorCamera.getViewMatrix());
            animShader.setVec3("lightDir", shadowManager.m_lightPos);
            animShader.setVec3("lightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
            animShader.setFloat("lightPower", lightPower);

            auto transforms = animator.m_FinalBoneMatrices;
            for (int i = 0; i < transforms.size(); ++i)
            {
                animShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            }

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, modelPosition);
            model = glm::rotate(model, modelRotate.x, glm::vec3(1, 0, 0));
            model = glm::rotate(model, modelRotate.y, glm::vec3(0, 1, 0));
            model = glm::rotate(model, modelRotate.z, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(modelScale, modelScale, modelScale));
            animShader.setMat4("model", model);
            animModel.draw(animShader);
        }

        // Render scene
        glm::vec4 frustumDistances = shadowManager.getFrustumDistances();

        terrain.drawColor(terrainShader, shadowManager.m_lightPos, glm::vec3(lightColor[0], lightColor[1], lightColor[2]),
                          lightPower,
                          editorCamera.getViewMatrix(), projection,
                          shadowmapManager.m_textureArray,
                          editorCamera.position, editorCamera.front,
                          frustumDistances,
                          editorCamera.position,
                          editorCamera.projectionMode == ProjectionMode::Ortho);

        terrain.drawInstance(grassShader, &grass, terrain.m_grassTileSize, terrain.m_grassDensity, projection, editorCamera.getViewMatrix(), editorCamera.position);
        terrain.drawInstance(stoneShader, &stone, terrain.m_stoneTileSize, terrain.m_stoneDensity, projection, editorCamera.getViewMatrix(), editorCamera.position);

        // Draw objects
        {
            glm::vec3 objectColor(0.6f, 0.6f, 0.6f);

            simpleShadow.use();
            simpleShadow.setFloat("biasMult", bias);
            simpleShadow.setVec3("AmbientColor", glm::vec3(ambientColor[0], ambientColor[1], ambientColor[2]));
            simpleShadow.setVec3("DiffuseColor", objectColor);
            simpleShadow.setVec3("SpecularColor", glm::vec3(specularColor[0], specularColor[1], specularColor[2]));
            simpleShadow.setVec3("LightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
            simpleShadow.setFloat("LightPower", lightPower);
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
            position = groundPos;
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
        lightPositions[0].x = radius * glm::sin(currentFrame * speed);
        lightPositions[0].y = radius * glm::sin(currentFrame * (speed / 6)) + 20;
        lightPositions[0].z = radius * glm::cos(currentFrame * speed) + 6;

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
        simpleShader.setVec4("DiffuseColor", glm::vec4(lightColor[0], lightColor[1], lightColor[2], 1.0f));
        sphere.draw(simpleShader);

        // Draw physics debug lines
        // TODO: own shader with only lines
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
        for (int i = 0; drawFrustum && i < shadowManager.m_splitCount; i++)
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
            glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), shadowManager.m_frustums[i].points, GL_STATIC_DRAW);

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
        for (int i = 0; drawAABB && i < shadowManager.m_splitCount; i++)
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
            glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), shadowManager.m_frustums[i].lightAABB, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, c_ebo_2);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(float), indices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

            glm::mat4 mvp = projection * editorCamera.getViewMatrix() * inverseDepthViewMatrix;
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

        // Post process
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);

        {
            postProcessShader.use();
            postProcessShader.setVec2("screenSize", glm::vec2((float)screenWidth, (float)screenHeight));
            postProcessShader.setFloat("blurOffset", blurOffset);
            postProcessShader.setFloat("exposure", postProcess.m_exposure);

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(postProcessShader.id, "renderedTexture"), 0);
            glBindTexture(GL_TEXTURE_2D, postProcess.m_texture);

            glBindVertexArray(q_vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        // Draw shadowmap
        if (drawShadowmap)
        {
            textureArrayShader.use();

            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(textureArrayShader.id, "renderedTexture"), 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, shadowmapManager.m_textureArray);

            for (int i = 0; i < shadowManager.m_splitCount; i++)
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

        // Render UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        showOverlay(&characterController, &animator, characterBody, &postProcess, &shadowManager, &editorCamera, &vehicle, &soundEngine, &terrain, &debugDrawer, soundSource, deltaTime, &show_overlay);
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
