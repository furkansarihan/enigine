#ifndef character_task_hpp
#define character_task_hpp

#include <string>

class CharacterTask
{
public:
    virtual ~CharacterTask() {}

    bool m_interrupted = false;

    virtual bool update() = 0;
    virtual void interrupt() = 0;
};

#endif /* character_task_hpp */
