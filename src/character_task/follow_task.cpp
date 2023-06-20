#include "follow_task.h"

FollowTask::FollowTask(Character *source, Character *destination)
    : m_source(source),
      m_destination(destination)
{
}

FollowTask::~FollowTask()
{
}

void FollowTask::update()
{
    float distance = glm::distance2(m_source->m_position, m_destination->m_position);
    float closeTreshold = 10.f;
    if (distance < closeTreshold)
    {
        m_source->m_controller->m_actionState.forward = false;
        m_source->m_controller->m_actionState.run = false;
        return;
    }

    glm::vec3 followDir = m_destination->m_position - m_source->m_position;
    followDir = glm::normalize(glm::vec3(followDir.x, 0, followDir.z));

    m_source->m_controller->m_refFront = followDir;
    m_source->m_controller->m_refRight = glm::cross(followDir, glm::vec3(0.f, 1.f, 0.f));
    m_source->m_controller->m_actionState.forward = true;

    float runTreshold = 30.f;
    if (distance > runTreshold)
        m_source->m_controller->m_actionState.run = true;
}
