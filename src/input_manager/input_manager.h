#ifndef input_manager_hpp
#define input_manager_hpp

#include <vector>

#include <GLFW/glfw3.h>

class InputManager
{
public:
    InputManager(GLFWwindow *window);
    ~InputManager();

    GLFWwindow *m_window;

    // TODO: addInputListener
    // TODO: cleaner approach
    void addKeyListener(const std::function<void(GLFWwindow *, int, int, int, int)> &listener);
    void addMouseButtonListener(const std::function<void(GLFWwindow *, int, int, int)> &listener);
    void addMouseScrollListener(const std::function<void(GLFWwindow *, int, int)> &listener);
    void addFileDropListener(const std::function<void(GLFWwindow *, int, const char **)> &listener);

private:
    std::vector<std::function<void(GLFWwindow *, int, int, int, int)>> m_keyListeners;
    std::vector<std::function<void(GLFWwindow *, int, int, int)>> m_mouseButtonListeners;
    std::vector<std::function<void(GLFWwindow *, int, int)>> m_mouseScrollListeners;
    std::vector<std::function<void(GLFWwindow *, int, const char **)>> m_fileDropListeners;

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void fileDropCallback(GLFWwindow *window, int count, const char **paths);
};

#endif /* input_manager_hpp */
