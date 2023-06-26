#include "follow_path.h"

FollowPath::FollowPath(Character *source, std::vector<glm::vec3> path, glm::vec3 lookDir)
    : m_source(source),
      m_path(path),
      m_lookDir(lookDir)
{
    m_source->m_controller->m_aimLocked = false;
}

FollowPath::~FollowPath()
{
}

void FollowPath::interrupt()
{
    m_interrupted = true;

    m_source->m_controller->m_actionState.forward = false;
    m_source->m_controller->m_actionState.run = false;
    m_source->m_controller->m_rotate = false;
    m_source->m_controller->m_walkFactor = 1.f;
}

// TODO: adaptive follow
bool FollowPath::update()
{
    if (m_interrupted)
        return true;

    if (m_orientationReady)
        return true;

    if (m_positionReady)
    {
        if (glm::distance2(m_source->m_controller->m_lookDir, m_lookDir) < 0.1f)
        {
            m_source->m_controller->m_rotate = false;
            m_orientationReady = true;
            return false;
        }

        m_source->m_controller->m_rotate = true;
        m_source->m_controller->m_refFront = m_lookDir;
        m_source->m_controller->m_refRight = glm::cross(m_lookDir, glm::vec3(0.f, 1.f, 0.f));

        return false;
    }

    if (m_path.size() == 0)
        return true;

    glm::vec3 nextPoint = m_path[0];
    bool reached = follow(nextPoint);

    if (reached)
    {
        m_path.erase(m_path.begin());
        glm::vec3 nextPoint = m_path[0];
        follow(nextPoint);
    }

    m_positionReady = m_path.size() == 0;

    if (m_positionReady)
    {
        m_source->m_controller->m_actionState.forward = false;
        m_source->m_controller->m_actionState.run = false;
        m_source->m_controller->m_walkFactor = 1.f;
    }

    return false;
}

bool FollowPath::follow(glm::vec3 point)
{
    bool lastPoint = m_path.size() == 1;
    float distance = glm::distance(glm::vec2(m_source->m_position.x, m_source->m_position.z), glm::vec2(point.x, point.z));
    bool farAway = distance > 2.f;

    // TODO: variable threshold
    float closeThreshold = lastPoint ? 0.2f : 2.f;
    if (distance < closeThreshold)
        return true;

    glm::vec3 followDir = point - m_source->m_position;
    followDir = glm::normalize(glm::vec3(followDir.x, 0, followDir.z));

    m_source->m_controller->m_refFront = followDir;
    m_source->m_controller->m_refRight = glm::cross(followDir, glm::vec3(0.f, 1.f, 0.f));
    m_source->m_controller->m_actionState.forward = true;

    // TODO: can be slower while turning
    m_source->m_controller->m_actionState.run = farAway;
    // slowly snap at last point
    if (lastPoint && distance < 1.f)
        m_source->m_controller->m_walkFactor = distance;
    else
        m_source->m_controller->m_walkFactor = 1.f;

    return false;
}
