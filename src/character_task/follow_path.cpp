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

void FollowPath::exit()
{
    m_source->m_controller->m_actionState.forward = false;
    m_source->m_controller->m_actionState.run = false;
    m_source->m_controller->m_rotate = false;
}

// TODO: self destroy with return true
// TODO: create an EnterVehicle task - chain of responsibilty?
void FollowPath::update()
{
    if (m_orientationReady)
        return;

    if (m_positionReady)
    {
        if (glm::distance2(m_source->m_controller->m_lookDir, m_lookDir) < 0.1f)
        {
            m_source->m_controller->m_rotate = false;
            m_orientationReady = true;
            return;
        }

        m_source->m_controller->m_rotate = true;
        m_source->m_controller->m_refFront = m_lookDir;

        return;
    }

    if (m_path.size() == 0)
        return;

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
    }
}

bool FollowPath::follow(glm::vec3 point)
{
    bool lastPoint = m_path.size() == 1;
    float distance = glm::distance(m_source->m_position, point);
    bool farAway = distance > 2.f;

    // TODO: interpolate to exact location - last point
    // TODO: variable treshold
    float closeTreshold = lastPoint ? 0.5f : 2.f;
    if (distance < closeTreshold)
        return true;

    glm::vec3 followDir = point - m_source->m_position;
    followDir = glm::normalize(glm::vec3(followDir.x, 0, followDir.z));

    m_source->m_controller->m_refFront = followDir;
    m_source->m_controller->m_refRight = glm::cross(followDir, glm::vec3(0.f, 1.f, 0.f));
    m_source->m_controller->m_actionState.forward = true;

    // TODO: can be slower while turning
    m_source->m_controller->m_actionState.run = farAway;

    return false;
}
