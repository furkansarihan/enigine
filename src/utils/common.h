#ifndef common_hpp
#define common_hpp

#include <iostream>
#include <string>
#include <mach/mach.h>

#include <GL/glew.h>

class CommonUtil
{
public:
    static inline void glfwErrorCallback(int error, const char *description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

    static inline void printStartInfo()
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
    static inline void refreshSystemMonitor(task_basic_info &t_info)
    {
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        task_info(mach_task_self(),
                  TASK_BASIC_INFO, (task_info_t)&t_info,
                  &t_info_count);
    }

    static inline float snappedClamp(float value, float min, float max, float snapLength)
    {
        float clamped = std::max(min, std::min(value, max));
        if (clamped > (max - snapLength))
            clamped = max;
        if (clamped < (min + snapLength))
            clamped = min;

        return clamped;
    }

    // TODO: primitive creation
    static inline void createQuad(unsigned int &vbo, unsigned int &vao, unsigned int &ebo)
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
};

#endif /* common_hpp */
