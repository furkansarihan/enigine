#include <iostream>
#include <ctime>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void print_start_info()
{
    // current date/time based on current system
    time_t now = time(0);
    char *date_time = ctime(&now);
    // version, format -> x.xx.xxx
    std::string version("000001");

    std::cout << "started_at: " << date_time << std::endl;
    std::cout << "enigine_version: " << version << std::endl;
    std::cout << "cpp_version: " << __cplusplus << std::endl;
}

int main(int argc, char **argv)
{
    print_start_info();

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
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
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
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

    int screen_width, screen_height;
    glfwGetFramebufferSize(window, &screen_width, &screen_height);
    glViewport(0, 0, screen_width, screen_height);

    // TODO: Create geometries

    // TODO: Init shader

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO: render geometries

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glfwSwapBuffers(window);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}