#ifndef follow_task_hpp
#define follow_task_hpp

#include <iostream>
#include <string>

#include "character_task.h"
class Character;
class NPCharacter;
class PCharacter;
#include "../character/character.h"

class FollowTask : public CharacterTask
{
public:
    FollowTask(Character *source, Character *destination);
    ~FollowTask();

    Character *m_source;
    Character *m_destination;

    void interrupt();
    bool update();
};

#endif /* follow_task_hpp */
