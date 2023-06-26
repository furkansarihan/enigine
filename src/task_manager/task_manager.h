#ifndef task_manager_hpp
#define task_manager_hpp

#include <iostream>
#include <string>
#include <vector>
#include <stack>

#include "../character_task/character_task.h"

class TaskManager
{
public:
    TaskManager();
    ~TaskManager();

    void update();
    void addTask(CharacterTask *task);
    void addTaskStack(std::stack<CharacterTask *> taskStack);
    std::vector<CharacterTask *> getTaskPointers(std::function<bool(CharacterTask *)> callback) const;

private:
    std::vector<CharacterTask *> m_tasks;
    std::vector<std::stack<CharacterTask *>> m_taskStacks;

    void updateTasks(std::vector<CharacterTask *> &list);
    void updateTaskStacks();
};

#endif /* task_manager_hpp */
