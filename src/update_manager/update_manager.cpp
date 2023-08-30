#include "update_manager.h"

UpdateManager::UpdateManager()
{
}

UpdateManager::~UpdateManager()
{
}

void UpdateManager::update(float deltaTime)
{
    for (Updatable *updatable : m_updatables)
        updatable->update(deltaTime);
}

void UpdateManager::add(Updatable *updatable)
{
    m_updatables.push_back(updatable);
}

void UpdateManager::remove(Updatable *updatable)
{
    auto it = std::find(m_updatables.begin(), m_updatables.end(), updatable);
    if (it != m_updatables.end())
        m_updatables.erase(it);
}
