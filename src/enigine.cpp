#include <iostream>
#include <ctime>
#include <bit>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <AL/al.h>
#include <AL/alc.h>

#include "external/imgui/imgui.h"
#define DR_WAV_IMPLEMENTATION
#include "external/dr_wav/dr_wav.h"

#include "shader/shader.h"
#include "file_manager/file_manager.h"
#include "camera/camera.h"
#include "model/model.h"

#define alCall(function, ...) alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(function, device, ...) alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

bool check_al_errors(const std::string &filename, const std::uint_fast32_t line)
{
    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error)
        {
        case AL_INVALID_NAME:
            std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function";
            break;
        case AL_INVALID_ENUM:
            std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function";
            break;
        case AL_INVALID_VALUE:
            std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case AL_INVALID_OPERATION:
            std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
            break;
        case AL_OUT_OF_MEMORY:
            std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory";
            break;
        default:
            std::cerr << "UNKNOWN AL ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template <typename alFunction, typename... Params>
auto alCallImpl(const char *filename,
                const std::uint_fast32_t line,
                alFunction function,
                Params... params)
    -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, decltype(function(params...))>
{
    auto ret = function(std::forward<Params>(params)...);
    check_al_errors(filename, line);
    return ret;
}

template <typename alFunction, typename... Params>
auto alCallImpl(const char *filename,
                const std::uint_fast32_t line,
                alFunction function,
                Params... params)
    -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_al_errors(filename, line);
}

bool check_alc_errors(const std::string &filename, const std::uint_fast32_t line, ALCdevice *device)
{
    ALCenum error = alcGetError(device);
    if (error != ALC_NO_ERROR)
    {
        std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
        switch (error)
        {
        case ALC_INVALID_VALUE:
            std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function";
            break;
        case ALC_INVALID_DEVICE:
            std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function";
            break;
        case ALC_INVALID_CONTEXT:
            std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function";
            break;
        case ALC_INVALID_ENUM:
            std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function";
            break;
        case ALC_OUT_OF_MEMORY:
            std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function";
            break;
        default:
            std::cerr << "UNKNOWN ALC ERROR: " << error;
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}

template <typename alcFunction, typename... Params>
auto alcCallImpl(const char *filename,
                 const std::uint_fast32_t line,
                 alcFunction function,
                 ALCdevice *device,
                 Params... params)
    -> typename std::enable_if_t<std::is_same_v<void, decltype(function(params...))>, bool>
{
    function(std::forward<Params>(params)...);
    return check_alc_errors(filename, line, device);
}

template <typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char *filename,
                 const std::uint_fast32_t line,
                 alcFunction function,
                 ReturnType &returnValue,
                 ALCdevice *device,
                 Params... params)
    -> typename std::enable_if_t<!std::is_same_v<void, decltype(function(params...))>, bool>
{
    returnValue = function(std::forward<Params>(params)...);
    return check_alc_errors(filename, line, device);
}

bool firstMove = true;
float lastX;
float lastY;

// Light source position
glm::vec3 lightPosition = glm::vec3(-3.0f, 7.0f, 6.0f);
// Light color
static float lightColor[3] = {1.0f, 1.0f, 0.9f};
// Light power
static float lightPower = 70.0;

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

static void showOverlay(Camera *editorCamera, ALuint source, float deltaTime, bool *p_open)
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
        ImGui::Separator();
        ImGui::Text("Camera");
        ImGui::Text("position: (%.1f, %.1f, %.1f)", editorCamera->position.x, editorCamera->position.y, editorCamera->position.z);
        ImGui::Text("pitch: %.1f", editorCamera->pitch);
        ImGui::Text("yaw: %.1f", editorCamera->yaw);
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
        ALint state = AL_PLAYING;
        alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);

        if (state == AL_PLAYING)
        {
            if (ImGui::Button("state: playing"))
            {
                alCall(alSourcePause, source);
            }
        }
        else if (state == AL_PAUSED)
        {
            if (ImGui::Button("state: paused"))
            {
                alCall(alSourcePlay, source);
            }
        }
        else if (state == AL_STOPPED)
        {
            if (ImGui::Button("state: stopped"))
            {
                alCall(alSourcePlay, source);
            }
        }
        else
        {
            ImGui::Text("state: unknown");
        }
        ImGui::SameLine();
        if (ImGui::Button("reset"))
        {
            alCall(alSourcef, source, AL_GAIN, 1.0f);
            alCall(alSourcef, source, AL_PITCH, 1.0f);
        }

        ALfloat gain;
        alCall(alGetSourcef, source, AL_GAIN, &gain);
        if (ImGui::SliderFloat("gain", &gain, 0.0f, 1.0f, "%.3f"))
        {
            alCall(alSourcef, source, AL_GAIN, gain);
        }

        ALfloat pitch;
        alCall(alGetSourcef, source, AL_PITCH, &pitch);
        if (ImGui::SliderFloat("pitch", &pitch, 0.5f, 2.0f, "%.3f"))
        {
            alCall(alSourcef, source, AL_PITCH, pitch);
        }

        ALint looping;
        alCall(alGetSourcei, source, AL_LOOPING, &looping);
        bool isLooping = looping == AL_TRUE;
        if (ImGui::Checkbox("looping", &isLooping))
        {
            if (looping == AL_TRUE)
            {
                alCall(alSourcei, source, AL_LOOPING, AL_FALSE);
            }
            else
            {
                alCall(alSourcei, source, AL_LOOPING, AL_TRUE);
            }
        }
        
    }
    ImGui::End();
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
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
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
    ALCdevice *openALDevice = alcOpenDevice(nullptr);
    if (!openALDevice)
    {
        std::cerr << "ERROR: Could not open audio device" << std::endl;
        return 0;
    }

    ALCcontext *openALContext;
    if (!alcCall(alcCreateContext, openALContext, openALDevice, openALDevice, nullptr) || !openALContext)
    {
        std::cerr << "ERROR: Could not create audio context" << std::endl;
        return 0;
    }
    ALCboolean contextMadeCurrent = false;
    if (!alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, openALContext) || contextMadeCurrent != ALC_TRUE)
    {
        std::cerr << "ERROR: Could not make audio context current" << std::endl;
        return 0;
    }

    // Read audio file
    drwav wav;
    if (!drwav_init_file(&wav, "assets/sounds/rain-loop-1648m.wav", NULL))
    {
        std::cerr << "ERROR: dr_wav could not init audio file" << std::endl;
        return 0;
    }

    std::cout << "Audio channels: " << wav.channels << std::endl;
    std::cout << "Audio sample rate: " << wav.sampleRate << std::endl;
    std::cout << "Audio total PCM Frame count: " << wav.totalPCMFrameCount << std::endl;
    std::cout << "Audio bits per sample: " << wav.bitsPerSample << std::endl;

    ALenum format;
    if (wav.channels == 1 && wav.bitsPerSample == 8)
        format = AL_FORMAT_MONO8;
    else if (wav.channels == 1 && wav.bitsPerSample == 16)
        format = AL_FORMAT_MONO16;
    else if (wav.channels == 2 && wav.bitsPerSample == 8)
        format = AL_FORMAT_STEREO8;
    else if (wav.channels == 2 && wav.bitsPerSample == 16)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr
            << "ERROR: unrecognised wave format: "
            << wav.channels << " channels, "
            << wav.bitsPerSample << " bitsPerSample" << std::endl;
        return 0;
    }

    // Create audio buffer
    ALuint buffer;
    alCall(alGenBuffers, 1, &buffer);

    if (format == AL_FORMAT_MONO8 || format == AL_FORMAT_STEREO8)
    {
        int32_t size = (size_t)wav.totalPCMFrameCount * wav.channels * sizeof(int8_t);
        int8_t *pSampleData = (int8_t *)malloc(size);
        drwav_read_pcm_frames(&wav, wav.totalPCMFrameCount, pSampleData);
        alCall(alBufferData, buffer, format, pSampleData, size, wav.sampleRate);
        drwav_uninit(&wav);
    }
    else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_STEREO16)
    {
        int32_t size = (size_t)wav.totalPCMFrameCount * wav.channels * sizeof(int16_t);
        int16_t *pSampleData = (int16_t *)malloc(size);
        drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, pSampleData);
        alCall(alBufferData, buffer, format, pSampleData, size, wav.sampleRate);
        drwav_uninit(&wav);
    }
    
    // Init audio source
    ALuint source;
    alCall(alGenSources, 1, &source);
    alCall(alSourcef, source, AL_PITCH, 1);
    alCall(alSourcef, source, AL_GAIN, 1.0f);
    alCall(alSource3f, source, AL_POSITION, 0, 0, 0);
    alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
    alCall(alSourcei, source, AL_LOOPING, AL_TRUE);
    alCall(alSourcei, source, AL_BUFFER, buffer);
    alCall(alListener3f, AL_POSITION, 4.0f, 4.0f, 4.0f);

    alCall(alSourcePlay, source);

    // Create geometries
    Model cube("assets/models/cube.obj");
    Model sphere("assets/models/sphere.obj");

    // Init shader
    Shader normalShader;
    normalShader.init(FileManager::read("assets/shaders/normal-shader.vs"), FileManager::read("assets/shaders/normal-shader.fs"));

    Shader simpleShader;
    simpleShader.init(FileManager::read("assets/shaders/simple-shader.vs"), FileManager::read("assets/shaders/simple-shader.fs"));

    // Camera
    Camera *editorCamera = new Camera(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f), -135.0f, -30.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

    // Triangle model matrix
    glm::mat4 model = glm::mat4(1.0f);

    // Time
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

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

    while (!glfwWindowShouldClose(window))
    {
        // Poll events
        glfwPollEvents();

        // Process input
        processCameraInput(window, editorCamera, deltaTime);

        // Update audio listener
        alCall(alListener3f, AL_POSITION, editorCamera->position.x, editorCamera->position.y, editorCamera->position.z);
        listenerOrientation.clear();
        listenerOrientation.push_back(editorCamera->front.x);
        listenerOrientation.push_back(editorCamera->front.y);
        listenerOrientation.push_back(editorCamera->front.z);
        listenerOrientation.push_back(editorCamera->up.x);
        listenerOrientation.push_back(editorCamera->up.y);
        listenerOrientation.push_back(editorCamera->up.z);
        alCall(alListenerfv, AL_ORIENTATION, listenerOrientation.data());

        // Clear window
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update projection
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

        // Calculate deltaTime
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Render geometries
        glm::mat4 mvp = projection * editorCamera->getViewMatrix() * model; // Model-View-Projection matrix
        glm::mat4 mv = editorCamera->getViewMatrix() * model;
        glm::mat3 ModelView3x3Matrix = glm::mat3(mv);

        normalShader.use();
        normalShader.setMat4("MVP", mvp);
        normalShader.setMat4("M", model);
        normalShader.setMat4("V", editorCamera->getViewMatrix());
        normalShader.setMat3("MV3x3", ModelView3x3Matrix);
        normalShader.setVec3("LightPosition_worldspace", lightPosition);
        normalShader.setVec3("LightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));
        normalShader.setFloat("LightPower", lightPower);
        sphere.draw(normalShader);

        // Draw light source
        mvp = projection * editorCamera->getViewMatrix() * glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)), lightPosition);
        simpleShader.use();
        simpleShader.setMat4("MVP", mvp);
        simpleShader.setVec4("DiffuseColor", glm::vec4(lightColor[0], lightColor[1], lightColor[2], 1.0f));
        cube.draw(simpleShader);

        // Render UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        showOverlay(editorCamera, source, deltaTime, &show_overlay);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup OpenAL
    alCall(alDeleteSources, 1, &source);
    alCall(alDeleteBuffers, 1, &buffer);

    alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
    alcCall(alcDestroyContext, openALDevice, openALContext);

    ALCboolean closed;
    alcCall(alcCloseDevice, closed, openALDevice, openALDevice);

    // Cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // Cleanup glfw
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
