#include "particle_engine.h"

float randomFloat(float min, float max);

ParticleEngine::ParticleEngine(Camera *viewCamera)
    : m_viewCamera(viewCamera)
{
}

ParticleEngine::~ParticleEngine()
{
}

void ParticleEngine::update(float deltaTime)
{
    updateParticles(deltaTime);
    emitParticles(deltaTime);

    std::sort(m_particles.begin(), m_particles.end(), [](Particle a, Particle b)
              { return a.distance > b.distance; });

    // for (int i = 0; i < m_particles.size(); i++)
    // {
    //     Particle &particle = m_particles[i];
    //     std::cout << "sorted: index: " << i << ", distance: " << particle.distance << std::endl;
    // }
}

void ParticleEngine::updateParticles(float deltaTime)
{
    for (int i = 0; i < m_particles.size();)
    {
        // update
        Particle &particle = m_particles[i];
        particle.duration -= deltaTime;
        particle.position += particle.velocity * deltaTime;
        particle.distance = glm::distance(m_viewCamera->position, particle.position);

        // velocity change?

        if (particle.duration < 0.f)
            m_particles.erase(m_particles.begin() + i);
        else
            ++i;
    }
}

void ParticleEngine::emitParticles(float deltaTime)
{
    int newParticle = deltaTime * m_particlesPerSecond;

    // if 0 - emit with probabilty
    if (newParticle == 0)
    {
        float particleProbabilty = deltaTime * m_particlesPerSecond;
        float probabilty = randomFloat(0.f, 1.f);
        if (probabilty < particleProbabilty)
        {
            newParticle = 1;
        }
    }

    for (int i = 0; i < newParticle; i++)
    {
        float duration = randomFloat(m_minDuration, m_maxDuration);
        float velocityForce = randomFloat(m_minVelocity, m_maxVelocity);

        glm::vec3 randomVector = glm::sphericalRand(1.0f);
        glm::vec3 velocityDirection = glm::normalize(m_direction + m_randomness * randomVector);

        m_particles.push_back(Particle(m_position, m_position, velocityDirection * velocityForce, duration));
    }
}

// TODO: instancing - compute shaders
void ParticleEngine::drawParticles(Shader *shader, Model *quad, glm::mat4 viewProjection)
{
    if (m_particles.empty())
        return;

    shader->use();
    shader->setFloat("particleScale", m_particleScale);
    for (int i = 0; i < m_particles.size(); i++)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, m_particles[i].position);

        // bilboarding
        glm::vec3 camToParticle = glm::normalize(m_particles[i].position - m_viewCamera->position);
        glm::vec3 rotation(0.f);
        rotation.y = glm::atan(camToParticle.x, camToParticle.z) + M_PI_2;
        rotation.z = -glm::atan(camToParticle.y, glm::length(glm::vec2(camToParticle.x, camToParticle.z)));
        // model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));

        model = glm::scale(model, glm::vec3(m_particleScale));

        shader->setFloat("emitDistance", glm::distance(m_particles[i].position, m_particles[i].emitPosition));
        shader->setFloat("duration", m_particles[i].duration);
        shader->setMat4("MVP", viewProjection * model);
        quad->draw(*shader);
    }
}

float randomFloat(float min, float max)
{
    float random = ((float)rand()) / (float)RAND_MAX;
    return min + (random * (max - min));
}
