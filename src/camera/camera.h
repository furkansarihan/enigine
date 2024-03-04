#ifndef camera_hpp
#define camera_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

enum ProjectionMode
{
    Perspective,
    Ortho,
};

class Camera
{
public:
    // Contructors
    Camera(glm::vec3 position, glm::vec3 up, float near = 0.1f, float far = 20000.0f);
    ~Camera();
    // Camera Attributes
    glm::vec3 position;
    glm::vec3 front = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    ProjectionMode projectionMode = ProjectionMode::Perspective;
    glm::vec3 frustumPoints[8];
    float m_near;
    float m_far;
    float fov = M_PI_4;
    float scaleOrtho = 1.f;
    // Camera options
    float movementSpeed = 25.0f;
    float mouseSensitivity = 0.002f;
    // Move
    bool moving = true;
    float lastX;
    float lastY;

    // Functions
    glm::mat4 getViewMatrix();
    glm::mat4 getViewMatrix(glm::vec3 worldOrigin);
    glm::mat4 getProjectionMatrix(float width, float height);
    void processInput(GLFWwindow *window, float deltaTime);
    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void updateFrustumPoints(float width, float height);
};

#endif /* camera_hpp */
