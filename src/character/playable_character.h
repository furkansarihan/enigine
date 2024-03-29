#ifndef playable_character_hpp
#define playable_character_hpp

#include <random>

#include "character.h"
#include "../sound_engine/sound_engine.h"
#include "../particle_engine/particle_engine.h"
#include "../utils/common.h"
#include "../shader_manager/shader_manager.h"

class PCharacter : public Character
{
public:
    PCharacter(ShaderManager *shaderManager, RenderManager *renderManager, SoundEngine *soundEngine, GLFWwindow *window, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~PCharacter();

    SoundEngine *m_soundEngine;
    // TODO: input manager
    GLFWwindow *m_window;
    std::vector<Character *> m_npcList;
    ALuint m_fireSoundBuffer;
    std::vector<SoundSource> m_fireSounds;

    bool m_firstPerson = false;

    glm::vec3 m_followOffsetAim;
    glm::vec3 m_followOffsetNormal;
    glm::vec3 m_followOffset;
    glm::vec3 m_followOffsetTarget;
    float m_followOffsetFactor;

    glm::mat4 m_pistolModelMatrix = glm::mat4(1.f);
    // TODO: bound to pistol model
    glm::vec3 m_pistolOffset = glm::vec3(1.1f, 16.2f, 3.4f);
    glm::quat m_pistolOrientation = glm::quat(0.563f, -0.432f, -0.447f, -0.545f); // 0, -76, -88
    float m_pistolScale = 100.f;

    glm::vec3 m_muzzlePosition = glm::vec3(0.f);
    glm::vec3 m_muzzleDirection = glm::vec3(0.f);
    glm::vec3 m_muzzleOffset = glm::vec3(4.6f, 10.8f, 2.7f);

    ParticleEngine *m_smokeParticle;
    ParticleEngine *m_muzzleFlash;
    RenderParticleSource *m_smokeSource;
    RenderParticleSource *m_muzzleSource;
    Shader m_smokeShader;
    Shader m_muzzleFlashShader;

    Model *m_pistolModel;
    RenderSource *m_pistolSource;

    float m_lastFire = 0.f;
    glm::vec3 m_lastFireHit = glm::vec3(0.f);
    int m_lastFireSound = -1;
    float m_fireLimit = 0.5f;
    float m_fireAnimStartTime = 1.f;
    int m_fireAnimTimeMilli = 100;

    float m_lastCarRequest = 0.f;
    float m_carRequestLimit = 1.f;

    void update(float deltaTime);
    void updatePistolModelMatrix();
    void fireWeapon();
    void shootRay();
    void playFireSound();

private:
    const int m_concurrentSoundCount = 6;
};

#endif /* playable_character_hpp */
