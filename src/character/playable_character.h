#ifndef playable_character_hpp
#define playable_character_hpp

#include <random>

#include "character.h"
#include "../sound_engine/sound_engine.h"
#include "../particle_engine/particle_engine.h"
#include "../utils/common.h"

class PCharacter : public Character
{
public:
    PCharacter(TaskManager *taskManager, SoundEngine *soundEngine, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera);
    ~PCharacter();

    TaskManager *m_taskManager;
    SoundEngine *m_soundEngine;
    std::vector<Character *> m_npcList;
    ALuint m_fireSoundBuffer;
    std::vector<SoundSource> m_fireSounds;

    bool m_firstPerson = false;
    bool m_controlCharacter = true;
    bool m_followCharacter = true;

    glm::vec3 m_followOffsetAim = glm::vec3(-1.2f, 3.4f, -2.3f);
    glm::vec3 m_followOffsetNormal = glm::vec3(0.0f, 3.f, -8.f);
    glm::vec3 m_followOffset = glm::vec3(0.0f, 3.f, -8.f);
    glm::vec3 m_followOffsetTarget = glm::vec3(0.0f, 3.f, -8.f);
    float m_followOffsetFactor = 0.1f;

    glm::mat4 m_pistolModel = glm::mat4(1.f);
    // TODO: bound to pistol model
    glm::vec3 m_pistolOffset = glm::vec3(1.1f, 16.2f, 3.4f);
    glm::quat m_pistolOrientation = glm::quat(0.563f, -0.432f, -0.447f, -0.545f); // 0, -76, -88
    float m_pistolScale = 100.f;

    glm::vec3 m_muzzlePosition = glm::vec3(0.f);
    glm::vec3 m_muzzleDirection = glm::vec3(0.f);
    glm::vec3 m_muzzleOffset = glm::vec3(4.6f, 10.8f, 2.7f);

    ParticleEngine *m_smokeParticle;
    ParticleEngine *m_muzzleFlash;

    float m_lastFire = 0.f;
    glm::vec3 m_lastFireHit = glm::vec3(0.f);
    int m_lastFireSound = -1;
    float m_fireLimit = 0.5f;
    float m_fireAnimStartTime = 1.f;
    int m_fireAnimTimeMilli = 100;

    float m_lastCarEnterRequest = 0.f;
    float m_carEnterRequestLimit = 1.f;

    void update(GLFWwindow *window, float deltaTime);
    void updatePistolModelMatrix();
    void fireWeapon();
    void shootRay();
    void playFireSound();

private:
    const int m_concurrentSoundCount = 6;
};

#endif /* playable_character_hpp */
