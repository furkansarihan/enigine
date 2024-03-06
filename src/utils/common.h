#ifndef common_hpp
#define common_hpp

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>

#include <GL/glew.h>
#include <glm/glm.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <mach-o/dyld.h>
#endif

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
        std::string version("0.0.1");

        std::cout << "enigine_version: " << version << std::endl;
        std::cout << "cpp_version: " << __cplusplus << std::endl;
        std::cout << "started_at: " << dateTime << std::endl;
    }

    static inline uint64_t getRamUsage()
    {
#ifdef __APPLE__
        task_basic_info_data_t t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        if (task_info(mach_task_self(),
                      TASK_BASIC_INFO,
                      (task_info_t)&t_info,
                      &t_info_count) != KERN_SUCCESS)
        {
            // Handle error if task_info fails
            return 0; // Return 0 indicating failure
        }

        // Return the resident size of the task in bytes
        return t_info.resident_size;
#elif _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            // Handle error if GetProcessMemoryInfo fails
            return 0; // Return 0 indicating failure
        }

        // Return the working set size of the process in bytes
        return pmc.WorkingSetSize;
#elif __linux__
        long rss = 0L;
        FILE *fp = NULL;
        if ((fp = fopen("/proc/self/statm", "r")) == NULL)
            return (size_t)0L; /* Can't open? */
        if (fscanf(fp, "%*s%ld", &rss) != 1)
        {
            fclose(fp);
            return (size_t)0L; /* Can't read? */
        }
        fclose(fp);
        return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
#else
        // Platform not supported
        return 0; // Return 0 indicating failure
#endif
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

    static inline float lerp(float x, float y, float t)
    {
        return x * (1.f - t) + y * t;
    }

    static inline float lerpAngle(float startAngle, float targetAngle, float t)
    {
        float shortestAngle = targetAngle - startAngle;

        // Wrap the angle to the range -π to π
        if (shortestAngle > M_PI)
            shortestAngle -= 2 * M_PI;
        else if (shortestAngle < -M_PI)
            shortestAngle += 2 * M_PI;

        // Perform linear interpolation
        float result = startAngle + t * shortestAngle;

        // Wrap the result to the range -π to π
        if (result > M_PI)
            result -= 2 * M_PI;
        else if (result < -M_PI)
            result += 2 * M_PI;

        return result;
    }

    static inline glm::vec2 cubicBezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, double t)
    {
        double x = pow(1 - t, 3) * p0.x + 3 * t * pow(1 - t, 2) * p1.x + 3 * pow(t, 2) * (1 - t) * p2.x + pow(t, 3) * p3.x;
        double y = pow(1 - t, 3) * p0.y + 3 * t * pow(1 - t, 2) * p1.y + 3 * pow(t, 2) * (1 - t) * p2.y + pow(t, 3) * p3.y;
        return glm::vec2(x, y);
    }

    // Bresenham's line
    static inline std::vector<glm::vec2> getLinePoints(glm::vec2 from, glm::vec2 to)
    {
        std::vector<glm::vec2> points;
        int x0 = from.x;
        int y0 = from.y;
        int x1 = to.x;
        int y1 = to.y;

        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sgnX = x0 < x1 ? 1 : -1;
        int sgnY = y0 < y1 ? 1 : -1;
        int e = 0;
        for (int i = 0; i < dx + dy; i++)
        {
            points.push_back(glm::vec2(x0, y0));
            int e1 = e + dy;
            int e2 = e - dx;
            if (abs(e1) < abs(e2))
            {
                x0 += sgnX;
                e = e1;
            }
            else
            {
                y0 += sgnY;
                e = e2;
            }
        }

        return points;
    }

    static glm::vec3 positionFromModel(glm::mat4 model)
    {
        glm::vec3 position;
        position.x = model[3][0];
        position.y = model[3][1];
        position.z = model[3][2];
        return position;
    }

    // https://stackoverflow.com/a/68323550
    static void decomposeModel(const glm::mat4 &m, glm::vec3 &pos, glm::quat &rot, glm::vec3 &scale)
    {
        pos = m[3];
        for (int i = 0; i < 3; i++)
            scale[i] = glm::length(glm::vec3(m[i]));
        const glm::mat3 rotMtx(
            glm::vec3(m[0]) / scale[0],
            glm::vec3(m[1]) / scale[1],
            glm::vec3(m[2]) / scale[2]);
        rot = glm::quat_cast(rotMtx);
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
        unsigned int quad_indices[] = {0, 2, 1, 1, 2, 3};
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

    static std::string getExecutablePath()
    {
        std::string path;
        size_t lastSeparator;

#if defined(_WIN32) || defined(_WIN64)
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        path = buffer;
        lastSeparator = path.find_last_of("\\");
#elif defined(__linux__)
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            path = buffer;
        }
        lastSeparator = path.find_last_of("/");
#elif defined(__APPLE__)
        char buffer[PATH_MAX];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
        {
            path = buffer;
        }
        lastSeparator = path.find_last_of("/");
#endif

        return path.substr(0, lastSeparator);
    }
};

#endif /* common_hpp */
