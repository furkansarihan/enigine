#ifndef exit_car_hpp
#define exit_car_hpp

#include <iostream>
#include <string>

#include "character_task.h"
class Character;
class NPCharacter;
class PCharacter;
#include "../character/character.h"
#include "../car_controller/car_controller.h"

class ExitCar : public CharacterTask
{
public:
    ExitCar(Character *character, CarController *car);
    ~ExitCar();

    Character *m_character;
    CarController *m_car;
    bool m_jumping;

    glm::vec3 m_refPos;
    glm::vec3 m_lookDir;
    float m_stateChangeSpeed = 0.02f;
    float m_posFactor = 0.05f;

    bool m_firstUpdate = false;
    bool m_sitting = false;
    bool m_positionReached = false;

    bool m_doorOpened = false;
    bool m_doorClosed = false;

    float m_doorOpenTime;
    float m_doorCloseTime;
    float m_minInterruptTime;

    bool m_startAnimBlendReady = false;
    bool m_animBlendReady = false;
    bool m_animFinished = false;
    bool m_interruptRequested = false;

    void interrupt();
    bool update();
    void interruptUpdate();
    void endTask();
    void updateRefValues();
    int exitAnimIndex();
    int exitPoseIndex();
};

#endif /* exit_car_hpp */
