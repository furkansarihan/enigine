#include "input_manager.h"

InputManager::InputManager(GLFWwindow *window)
    : m_window(window)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetDropCallback(window, fileDropCallback);
}

InputManager::~InputManager()
{
    glfwSetKeyCallback(m_window, NULL);
    glfwSetMouseButtonCallback(m_window, NULL);
    glfwSetScrollCallback(m_window, NULL);
    glfwSetDropCallback(m_window, NULL);
}

void InputManager::addKeyListener(const std::function<void(GLFWwindow *, int, int, int, int)> &listener)
{
    m_keyListeners.push_back(listener);
}

void InputManager::addMouseButtonListener(const std::function<void(GLFWwindow *, int, int, int)> &listener)
{
    m_mouseButtonListeners.push_back(listener);
}

void InputManager::addMouseScrollListener(const std::function<void(GLFWwindow *, int, int)> &listener)
{
    m_mouseScrollListeners.push_back(listener);
}

void InputManager::addFileDropListener(const std::function<void(GLFWwindow *, int, const char **)> &listener)
{
    m_fileDropListeners.push_back(listener);
}

void InputManager::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (!inputManager)
        return;

    for (const auto &listener : inputManager->m_keyListeners)
    {
        listener(window, key, scancode, action, mods);
    }
}

void InputManager::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (!inputManager)
        return;

    for (const auto &listener : inputManager->m_mouseButtonListeners)
    {
        listener(window, button, action, mods);
    }
}

void InputManager::mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (!inputManager)
        return;

    for (const auto &listener : inputManager->m_mouseScrollListeners)
    {
        listener(window, xoffset, yoffset);
    }
}

void InputManager::fileDropCallback(GLFWwindow *window, int count, const char **paths)
{
    InputManager *inputManager = static_cast<InputManager *>(glfwGetWindowUserPointer(window));

    if (!inputManager)
        return;

    for (const auto &listener : inputManager->m_fileDropListeners)
    {
        listener(window, count, paths);
    }
}
