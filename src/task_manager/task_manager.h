#ifndef task_manager_hpp
#define task_manager_hpp

#include <iostream>
#include <string>
#include <vector>

class FollowTask;
#include "../character_task/follow_task.h"
class FollowPath;
#include "../character_task/follow_path.h"

class TaskManager
{
public:
    TaskManager();
    ~TaskManager();

    std::vector<FollowTask> m_tasksFollowCharacter;
    std::vector<FollowPath> m_tasksFollowPath;

    void update();
    void newFollowPath(FollowPath &followPath);
    void interruptFollowPath(void *character);
};

#endif /* task_manager_hpp */
