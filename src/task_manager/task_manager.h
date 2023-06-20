#ifndef task_manager_hpp
#define task_manager_hpp

#include <iostream>
#include <string>
#include <vector>

#include "../character/character.h"
#include "../character_task/follow_task.h"

class TaskManager
{
public:
    TaskManager();
    ~TaskManager();

    std::vector<FollowTask *> m_tasks;

    void update();
};

#endif /* task_manager_hpp */
