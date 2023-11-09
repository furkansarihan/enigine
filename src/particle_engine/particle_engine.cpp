#include "particle_engine.h"

float randomFloat(float min, float max);

ParticleEngine::ParticleEngine(ResourceManager *resourceManager, Model *particleCopy, Camera *viewCamera)
    : m_viewCamera(viewCamera)
{
    m_model = resourceManager->getModelFullPath(particleCopy->m_path, true);
    setupBuffer();
}

ParticleEngine::~ParticleEngine()
{
    delete m_model;
}

void ParticleEngine::setupBuffer()
{
    glGenBuffers(1, &m_arrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_arrayBuffer);

    for (size_t i = 0; i < m_model->meshes.size(); i++)
    {
        unsigned int VAO = m_model->meshes[i]->VAO;
        glBindVertexArray(VAO);

        float size = sizeof(Particle);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, size, (void *)0);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, size, (void *)offsetof(Particle, emitPosition));

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, size, (void *)offsetof(Particle, velocity));

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, size, (void *)offsetof(Particle, duration));

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, size, (void *)offsetof(Particle, maxDuration));

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, size, (void *)offsetof(Particle, distance));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);
    }

    glBindVertexArray(0);
}

void ParticleEngine::update(float deltaTime)
{
    emitParticles(deltaTime);
    updateParticles(deltaTime);

    // TODO: optional - when?
    // std::sort(m_particles.begin(), m_particles.end(), [](Particle a, Particle b)
    //           { return a.distance > b.distance; });

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

    glBindBuffer(GL_ARRAY_BUFFER, m_arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_particles.size() * sizeof(Particle), m_particles.data(), GL_STATIC_DRAW);
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

        m_particles.push_back(Particle(m_position, m_position, velocityDirection * velocityForce, duration, duration));
    }
}

// TODO: instancing - compute shaders
void ParticleEngine::drawParticles(Shader *shader, glm::mat4 viewProjection, glm::vec3 worldOrigin)
{
    if (m_particles.empty())
        return;

    shader->use();
    shader->setMat4("u_viewProjection", viewProjection);
    shader->setVec3("u_worldOrigin", worldOrigin);
    shader->setVec3("u_viewPosition", m_viewCamera->position);
    shader->setFloat("u_particleScale", m_particleScale);

    m_model->drawInstanced(*shader, m_particles.size());
}

float randomFloat(float min, float max)
{
    float random = ((float)rand()) / (float)RAND_MAX;
    return min + (random * (max - min));
}
