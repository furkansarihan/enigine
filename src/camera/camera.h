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

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 25.0f;
const float SENSITIVITY = 0.1f;
const float FOV = M_PI_4;
const float NEAR = 0.1f;
const float FAR = 20000.0f;

class Camera
{
public:
    // Contructors
    Camera(glm::vec3 position, glm::vec3 up, float yaw = YAW, float pitch = PITCH, float near = NEAR, float far = FAR);
    ~Camera();
    // Camera Attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    ProjectionMode projectionMode;
    glm::vec3 frustumPoints[8];
    float near;
    float far;
    float fov;
    float scaleOrtho;
    // Euler Angles
    float yaw;
    float pitch;
    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    // Move
    bool firstMove = true;
    float lastX;
    float lastY;

    // Functions
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix(float width, float height);
    void processInput(GLFWwindow *window, float deltaTime);
    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void updateFrustumPoints(float width, float height);

private:
    void updateCameraVectors();
};

#endif /* camera_hpp */
