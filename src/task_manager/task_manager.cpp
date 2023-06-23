#include "task_manager.h"

TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

void TaskManager::update()
{
    for (int i = 0; i < m_tasksFollowCharacter.size(); i++)
        m_tasksFollowCharacter[i].update();

    for (int i = 0; i < m_tasksFollowPath.size(); i++)
        m_tasksFollowPath[i].update();
}

void TaskManager::newFollowPath(FollowPath &followPath)
{
    for (int i = 0; i < m_tasksFollowPath.size(); i++)
    {
        if (m_tasksFollowPath[i].m_source == followPath.m_source)
            m_tasksFollowPath[i] = followPath;
    }

    m_tasksFollowPath.push_back(followPath);
}

// TODO: O(1)
void TaskManager::interruptFollowPath(void *character)
{
    int index = -1;
    for (int i = 0; i < m_tasksFollowPath.size(); i++)
    {
        if (m_tasksFollowPath[i].m_source == character)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        m_tasksFollowPath[index].exit();
        m_tasksFollowPath.erase(m_tasksFollowPath.begin() + index);
    }
}
