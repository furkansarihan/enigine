#include "task_manager.h"

TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

void TaskManager::update()
{
    updateTasks(m_tasks);
    updateTaskStacks();
}

void TaskManager::updateTasks(std::vector<CharacterTask *> &list)
{
    for (int i = 0; i < list.size(); i++)
    {
        if (!list[i])
        {
            list.erase(list.begin() + i);
            --i;
            continue;
        }

        bool result = list[i]->update();

        if (result)
        {
            delete list[i];
            list.erase(list.begin() + i);
            --i;
        }
    }
}

void TaskManager::updateTaskStacks()
{
    for (int i = 0; i < m_taskStacks.size(); i++)
    {
        std::stack<CharacterTask *> &taskStack = m_taskStacks[i];

        if (taskStack.size() == 0)
        {
            m_taskStacks.erase(m_taskStacks.begin() + i);
            --i;
            continue;
        }

        CharacterTask *task = taskStack.top();
        bool result = task->update();

        if (result)
        {
            delete task;
            taskStack.pop();
        }
    }
}

void TaskManager::addTask(CharacterTask *task)
{
    m_tasks.push_back(task);
}

void TaskManager::addTaskStack(std::stack<CharacterTask *> taskStack)
{
    m_taskStacks.push_back(taskStack);
}

// TODO: O(n)
std::vector<CharacterTask *> TaskManager::getTaskPointers(std::function<bool(CharacterTask *)> callback) const
{
    std::vector<CharacterTask *> taskPointers;

    for (CharacterTask *task : m_tasks)
    {
        if (callback(task))
            taskPointers.push_back(task);
    }

    for (const std::stack<CharacterTask *> &stack : m_taskStacks)
    {
        std::stack<CharacterTask *> tempStack = stack;
        while (!tempStack.empty())
        {
            CharacterTask *task = tempStack.top();
            if (callback(task))
                taskPointers.push_back(task);

            tempStack.pop();
        }
    }

    return taskPointers;
}
