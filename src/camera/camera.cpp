#include "camera.h"

// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch, float near, float far) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), fov(FOV), scaleOrtho(1.0), projectionMode(ProjectionMode::Perspective)
{
    this->position = position;
    this->worldUp = worldUp;
    this->yaw = yaw;
    this->pitch = pitch;
    this->near = near;
    this->far = far;
    updateCameraVectors();
}

Camera::~Camera()
{
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float width, float height)
{
    if (projectionMode == ProjectionMode::Ortho)
    {
        float halfW = (width / 2) * scaleOrtho;
        float halfH = (height / 2) * scaleOrtho;
        return glm::ortho(-halfW, halfW, -halfH, halfH, near, far);
    }
    else
    {
        return glm::perspective(fov, width / height, near, far);
    }
}

// TODO: EditorCamera
void Camera::processInput(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        processKeyboard(FORWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        processKeyboard(BACKWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        processKeyboard(LEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        processKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
        processMouseMovement(xoffset, yoffset, true);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
        firstMove = true;
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::processKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD)
        position += front * velocity;
    if (direction == BACKWARD)
        position -= front * velocity;
    if (direction == LEFT)
        position -= right * velocity;
    if (direction == RIGHT)
        position += right * velocity;
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainpitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainpitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // Update front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors()
{
    // Calculate the new front vector
    glm::vec3 _front;
    _front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    _front.y = sin(glm::radians(pitch));
    _front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(_front);
    // Also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    up = glm::normalize(glm::cross(right, front));
}

// TODO: reuse last
void Camera::updateFrustumPoints(float width, float height)
{
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    glm::vec3 right = normalize(cross(front, up));

    up = normalize(cross(right, front));

    glm::vec3 fc = position + front * far;
    glm::vec3 nc = position + front * near;

    float near_height;
    float near_width;
    float far_height;
    float far_width;

    if (projectionMode == ProjectionMode::Ortho)
    {
        near_height = (height / 2) * scaleOrtho;
        near_width = (width / 2) * scaleOrtho;
        far_height = (height / 2) * scaleOrtho;
        far_width = (width / 2) * scaleOrtho;
    }
    else
    {
        float ratio = width / height;
        near_height = tan(fov / 2.0f) * near;
        near_width = near_height * ratio;
        far_height = tan(fov / 2.0f) * far;
        far_width = far_height * ratio;
    }

    frustumPoints[0] = nc - up * near_height - right * near_width;
    frustumPoints[1] = nc + up * near_height - right * near_width;
    frustumPoints[2] = nc + up * near_height + right * near_width;
    frustumPoints[3] = nc - up * near_height + right * near_width;

    frustumPoints[4] = fc - up * far_height - right * far_width;
    frustumPoints[5] = fc + up * far_height - right * far_width;
    frustumPoints[6] = fc + up * far_height + right * far_width;
    frustumPoints[7] = fc - up * far_height + right * far_width;
}
