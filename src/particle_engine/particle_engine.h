#ifndef particle_engine_hpp
#define particle_engine_hpp

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "../shader/shader.h"
#include "../model/model.h"
#include "../camera/camera.h"

struct Particle
{
    glm::vec3 position;
    glm::vec3 emitPosition;
    glm::vec3 velocity;
    float duration;
    float distance;

    Particle(glm::vec3 position, glm::vec3 emitPosition, glm::vec3 velocity, float duration)
        : position(position), emitPosition(emitPosition), velocity(velocity), duration(duration) {}
};

class ParticleEngine
{
public:
    ParticleEngine(Camera *viewCamera);
    ~ParticleEngine();
    void update(float deltaTime);
    void drawParticles(Shader *shader, Model *quad, glm::mat4 viewProjection, glm::vec3 worldOrigin);

    // TODO: getAABB

    Camera *m_viewCamera;
    std::vector<Particle> m_particles;
    glm::vec3 m_position = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 m_direction = glm::vec3(0.f, 1.f, 0.f);
    float m_particlesPerSecond = 250.f;
    float m_randomness = 0.25f;
    float m_minVelocity = 0.1f;
    float m_maxVelocity = 1.0f;
    float m_minDuration = 1.0f;
    float m_maxDuration = 3.0f;
    float m_particleScale = 0.1f;

private:
    void updateParticles(float deltaTime);
    void emitParticles(float deltaTime);
};

#endif /* particle_engine_hpp */
