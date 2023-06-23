#ifndef follow_task_hpp
#define follow_task_hpp

#include <iostream>
#include <string>

class Character;
class NPCharacter;
class PCharacter;
#include "../character/character.h"

class FollowTask
{
public:
    FollowTask(Character *source, Character *destination);
    ~FollowTask();

    Character *m_source;
    Character *m_destination;

    void update();
};

#endif /* follow_task_hpp */
