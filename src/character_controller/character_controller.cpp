#include "character_controller.h"

CharacterController::CharacterController(btDiscreteDynamicsWorld *dynamicsWorld, btRigidBody *rigidBody, Camera *followCamera)
    : m_dynamicsWorld(dynamicsWorld),
      m_rigidBody(rigidBody),
      m_followCamera(followCamera)
{
    btCapsuleShape *shape = (btCapsuleShape *)m_rigidBody->getCollisionShape();
    m_halfHeight = shape->getHalfHeight() + shape->getRadius();

    m_walkSpeed.m_constantValue = 3.f;
    m_walkSpeed.m_points.push_back(LimiterPoint(M_PI, 0.7f, 1.f));  // back
    m_walkSpeed.m_points.push_back(LimiterPoint(2.0f, 1.5f, 0.1f)); // right-back spot
    m_walkSpeed.m_points.push_back(LimiterPoint(4.3f, 1.5f, 0.1f)); // left-back spot

    m_runSpeed.m_constantValue = 10.f;
    m_runSpeed.m_points.push_back(LimiterPoint(M_PI, 4.f, 1.f));
    m_runSpeed.m_points.push_back(LimiterPoint(2.0f, 4.f, 0.1f));
    m_runSpeed.m_points.push_back(LimiterPoint(4.3f, 4.f, 0.1f));
}

CharacterController::~CharacterController()
{
}

void CharacterController::updateElevation()
{
    btVector3 up(0, 1, 0);
    btVector3 from = m_rigidBody->getWorldTransform().getOrigin();
    btVector3 to = from - up * 10.0; // Example raycast length
    btCollisionWorld::ClosestRayResultCallback callback(from, to);
    m_dynamicsWorld->rayTest(from, to, callback);
    if (callback.hasHit())
    {
        m_worldElevation = callback.m_hitPointWorld.getY();
        m_elevationDistance = from.getY() - m_worldElevation;
        // btRigidBody *colliderBeneath = btRigidBody::upcast(callback.m_collisionObject);
    }
}

void CharacterController::updateVelocity()
{
    btVector3 origin(0, 0, 0);

    m_velocity = m_rigidBody->getLinearVelocity();
    m_speed = m_velocity.distance(origin);

    m_verticalVelocity = btVector3(m_velocity.getX(), 0, m_velocity.getZ());
    m_verticalSpeed = m_verticalVelocity.distance(origin);
    btVector3 horizontalVelocity = btVector3(0, m_velocity.getY(), 0);
    m_horizontalSpeed = horizontalVelocity.distance(origin);
}

void CharacterController::recieveInput(GLFWwindow *window, float deltaTime)
{
    m_actionState.forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    m_actionState.backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    m_actionState.left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    m_actionState.right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    m_actionState.run = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    m_actionState.jump = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

    m_refFront = glm::normalize(glm::vec3(m_followCamera->front.x, 0.0f, m_followCamera->front.z));
    m_refRight = glm::normalize(glm::vec3(m_followCamera->right.x, 0.0f, m_followCamera->right.z));
}

void CharacterController::update(float deltaTime)
{
    // TODO: exit if no input provided and speed is 0

    float prevElevation = m_elevationDistance;

    this->updateElevation();
    this->updateVelocity();

    m_onGround = m_elevationDistance < (m_halfHeight + m_groundThreshold);

    if (m_elevationDistance > prevElevation + 1.0f)
    {
        m_falling = true;
    }

    if (m_jumping && m_onGround)
    {
        m_jumping = false;
        m_speedAtJumpStart = 0.0f;
    }

    if (m_falling && m_onGround)
    {
        m_falling = false;
    }

    m_running = m_actionState.run;

    // jump
    if (m_actionState.jump && m_onGround && !m_jumping)
    {
        m_jumping = true;
        m_speedAtJumpStart = m_verticalSpeed;

        m_rigidBody->setActivationState(1);
        // m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
        m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
        float force = m_rigidBody->getMass() * m_jumpForce * deltaTime;
        m_rigidBody->applyCentralImpulse(btVector3(0, force, 0));
    }

    // move
    glm::vec3 moveTarget(0.0f, 0.0f, 0.0f);
    float frontForce = 0.0f;
    float rightForce = 0.0f;

    if (m_actionState.forward)
        frontForce += 1.0f;
    if (m_actionState.backward)
        frontForce -= 1.0f;
    if (m_actionState.left)
        rightForce -= 1.0f;
    if (m_actionState.right)
        rightForce += 1.0f;

    bool move = false;
    if (frontForce != 0.0f && !m_falling)
    {
        move = true;
        glm::vec3 frontXZ = m_refFront;
        glm::vec3 front = frontXZ * frontForce;
        moveTarget += front;
    }

    if (rightForce != 0.0f && !m_falling)
    {
        move = true;
        glm::vec3 rightXZ = m_refRight;
        glm::vec3 right = rightXZ * rightForce;
        moveTarget += right;
    }

    // TODO: prevent jump when move ends

    m_moving = move;

    // look
    glm::vec3 lookTarget;
    if (m_aimLocked || m_rotate)
        lookTarget = m_refFront;
    else
        lookTarget = glm::normalize(moveTarget);

    // turn
    float normalizedDistance = 1.f;
    if (m_moving || m_aimLocked || m_rotate)
    {
        float distance = std::abs(glm::distance(m_lookDir, lookTarget));
        // avoid 180 degree
        if (distance > 1.9f)
        {
            lookTarget = glm::rotateY(lookTarget, (float)M_PI_4);
            distance = std::abs(glm::distance(m_lookDir, lookTarget));
        }

        m_turning = distance > m_turnThreshold;
        normalizedDistance = ((2.0f - distance) / 2.0f);
        m_turnTarget = (1.0f - normalizedDistance) * m_turnAnimMaxFactor;

        if (m_turning)
        {
            // turn direction
            glm::mat2 mat = glm::mat2(
                glm::vec2(m_lookDir.x, lookTarget.x),
                glm::vec2(m_lookDir.z, lookTarget.z));
            m_det = glm::determinant(mat);
            m_turnTarget *= m_det > 0.0f ? 1.0f : -1.0f;

            float factor = m_turnForce * glm::abs(m_turnFactor);
            if (factor < m_minTurnFactor)
                factor = m_minTurnFactor;
            m_lookDir = glm::normalize(glm::mix(m_lookDir, lookTarget, factor));
        }
        else
            m_lookDir = lookTarget;
    }
    else
    {
        m_turnTarget = 0.0f;

        // TODO: better way?
        if (m_turnFactor == 0.0f)
            m_turning = false;
    }

    // turning animation
    if (glm::abs(m_turnFactor - m_turnTarget) > 0.01f)
        m_turnFactor = CommonUtil::lerp(m_turnFactor, m_turnTarget, m_turnAnimForce);
    else
        m_turnFactor = m_turnTarget;

    // move
    if (m_moving)
    {
        // TODO: calculate forceVec
        btVector3 forceVec = BulletGLM::getBulletVec3(moveTarget);
        float force = m_rigidBody->getMass() * m_moveForce * deltaTime;

        forceVec *= force;
        forceVec *= normalizedDistance;

        glm::vec3 moveDir = glm::normalize(BulletGLM::getGLMVec3(m_velocity));
        m_dotFront = glm::dot(m_lookDir, moveDir);

        m_signedMoveAngleTarget = glm::acos(m_dotFront);
        glm::vec3 cross = glm::cross(m_lookDir, moveDir);
        float crossDotAxis = glm::dot(cross, glm::vec3(0.f, 1.f, 0.f));
        if (crossDotAxis > 0.0f)
            m_signedMoveAngleTarget = -m_signedMoveAngleTarget;

        // max speed check
        m_maxWalkRelative = m_walkSpeed.getSpeed(m_signedMoveAngleTarget);
        m_maxRunRelative = m_runSpeed.getSpeed(m_signedMoveAngleTarget);

        bool moveBlocked = false;
        if (m_jumping && m_verticalSpeed > m_speedAtJumpStart)
            moveBlocked = true;
        else if (!m_running && m_verticalSpeed > m_maxWalkRelative * m_walkFactor)
            moveBlocked = true;
        else if (m_running && m_verticalSpeed > m_maxRunRelative)
            moveBlocked = true;

        if (!moveBlocked)
        {
            // TODO: has force
            m_rigidBody->setActivationState(1);
            m_rigidBody->applyCentralForce(forceVec);
        }
    }

    if (!m_moving)
        m_dotFront = 0.f;

    // interpolate move angle
    if (glm::abs(m_signedMoveAngle - m_signedMoveAngleTarget) > 0.01f)
        m_signedMoveAngle = CommonUtil::lerpAngle(m_signedMoveAngle, m_signedMoveAngleTarget, m_moveAngleForce);
    else
        m_signedMoveAngle = m_signedMoveAngleTarget;

    // slow down and stop
    if (m_verticalSpeed > 0 && !m_moving && !m_jumping && !m_falling)
    {
        float force = m_rigidBody->getMass() * m_toIdleForce * deltaTime;
        m_rigidBody->applyCentralForce(-m_verticalVelocity * force);
    }

    // TODO: remove teleport while walking on a rigidbody over terrain
    // snap to ground
    if (m_moving && !m_jumping && !m_falling)
    {
        // float force = m_rigidBody->getMass() * m_toIdleForceHoriz * deltaTime;
        // m_rigidBody->applyCentralForce(btVector3(0, -force * m_elevationDistance, 0));

        btVector3 pos = m_rigidBody->getWorldTransform().getOrigin();
        float gap = m_elevationDistance - m_halfHeight;
        float surface = pos.getY() - gap;
        btTransform transform;
        transform.setIdentity();
        // removes sliding caused by 0 friction - m_floatElevation
        transform.setOrigin(btVector3(pos.getX(), surface + m_floatElevation, pos.getZ()));
        m_rigidBody->setWorldTransform(transform);
        m_rigidBody->setLinearVelocity(btVector3(m_velocity.getX(), 0, m_velocity.getZ()));
    }
}
