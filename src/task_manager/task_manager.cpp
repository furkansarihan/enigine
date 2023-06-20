#include "task_manager.h"

TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

void TaskManager::update()
{
    for (int i = 0; i < m_tasks.size(); i++)
        m_tasks[i]->update();
}
