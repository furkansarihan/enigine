#ifndef enter_car_hpp
#define enter_car_hpp

#include <iostream>
#include <string>

#include "character_task.h"
class Character;
class NPCharacter;
class PCharacter;
#include "../character/character.h"
#include "../car_controller/car_controller.h"

class EnterCar : public CharacterTask
{
public:
    EnterCar(Character *character, CarController *car);
    ~EnterCar();

    Character *m_character;
    CarController *m_car;

    glm::vec3 m_refPos;
    glm::vec3 m_lookDir;
    float m_stateChangeSpeed = 0.01f;
    float m_posFactor = 0.01f;

    bool m_firstUpdate = false;
    bool m_sitting = false;
    bool m_positionReached = false;
    bool m_rotationReached = false;

    void interrupt();
    bool update();
    void updateRefValues();
};

#endif /* enter_car_hpp */
