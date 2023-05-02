#include "character_controller.h"

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t);
float lerp(float x, float y, float t);

CharacterController::CharacterController(btDiscreteDynamicsWorld *dynamicsWorld, btRigidBody *rigidBody, Camera *followCamera)
    : m_dynamicsWorld(dynamicsWorld),
      m_rigidBody(rigidBody),
      m_followCamera(followCamera)
{
    btCapsuleShape *shape = (btCapsuleShape *)m_rigidBody->getCollisionShape();
    m_halfHeight = shape->getHalfHeight() + shape->getRadius();
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
        m_elevationDistance = (callback.m_hitPointWorld - from).length();
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

void CharacterController::update(GLFWwindow *window, float deltaTime)
{
    float prevElevation = m_elevationDistance;

    this->updateElevation();
    this->updateVelocity();

    m_onGround = m_elevationDistance < (m_halfHeight + m_groundTreshold);

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

    m_running = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    // jump
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && m_onGround && !m_jumping)
    {
        m_jumping = true;
        m_speedAtJumpStart = m_verticalSpeed;

        m_rigidBody->setActivationState(1);
        // m_rigidBody->setLinearVelocity(btVector3(0, 0, 0));
        m_rigidBody->setAngularVelocity(btVector3(0, 0, 0));
        float force = m_rigidBody->getMass() * m_jumpForce * deltaTime;
        m_rigidBody->applyCentralImpulse(btVector3(0, force, 0));
    }

    // max speed check
    // TODO: blocking move input?
    if (m_jumping && m_verticalSpeed > m_speedAtJumpStart)
        return;
    else if (!m_running && m_verticalSpeed > m_maxWalkSpeed)
        return;
    else if (m_running && m_verticalSpeed > m_maxRunSpeed)
        return;

    // move
    btVector3 forceVec(0.0f, 0.0f, 0.0f);
    glm::vec3 moveVec(0.0f, 0.0f, 0.0f);
    float frontForce = 0.0f;
    float rightForce = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        frontForce += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        frontForce -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        rightForce += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        rightForce -= 1.0f;

    bool move = false;
    if (frontForce != 0.0f && !m_falling)
    {
        move = true;
        float force = m_rigidBody->getMass() * m_moveForce * deltaTime;
        glm::vec3 frontXZ = glm::normalize(glm::vec3(m_followCamera->front.x, 0.0f, m_followCamera->front.z));
        glm::vec3 front = frontXZ * frontForce;
        moveVec += front;
        forceVec += btVector3(front.x * force, 0.0f, front.z * force);
    }

    if (rightForce != 0.0f && !m_falling)
    {
        move = true;
        float force = m_rigidBody->getMass() * m_moveForce * deltaTime;
        glm::vec3 rightXZ = glm::normalize(glm::vec3(m_followCamera->right.x, 0.0f, m_followCamera->right.z));
        glm::vec3 right = rightXZ * rightForce;
        moveVec += right;
        forceVec += btVector3(right.x * force, 0.0f, right.z * force);
    }

    // TODO: prevent jump when move ends

    m_moving = move;

    if (m_moving)
    {
        moveVec = glm::normalize(moveVec);

        float distance = glm::distance(m_moveDir, moveVec);
        // avoid 180 degree
        if (distance == 2.0f)
        {
            moveVec = glm::rotateY(moveVec, (float)M_PI_4);
            distance = glm::distance(m_moveDir, moveVec);
        }

        m_turning = distance > m_turnTreshold;
        float normalizedDistance = ((2.0f - distance) / 2.0f);
        m_turnTarget = (1.0f - normalizedDistance) * m_turnAnimMaxFactor;

        if (m_turning)
        {
            // turn direction
            glm::mat2 mat = glm::mat2(
                glm::vec2(m_moveDir.x, moveVec.x),
                glm::vec2(m_moveDir.z, moveVec.z));
            float det = glm::determinant(mat);
            m_turnTarget *= det > 0.0f ? 1.0f : -1.0f;

            m_moveDir = glm::normalize(lerp(m_moveDir, moveVec, m_turnForce));
            forceVec *= normalizedDistance;
        }
        else
        {
            m_moveDir = moveVec;
        }

        m_rigidBody->setActivationState(1);
        m_rigidBody->applyCentralForce(forceVec);
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
        m_turnFactor = lerp(m_turnFactor, m_turnTarget, m_turnAnimForce);
    else
        m_turnFactor = m_turnTarget;

    // slow down and stop
    if (m_verticalSpeed > 0 && !m_moving && !m_jumping && !m_falling)
    {
        float force = m_rigidBody->getMass() * m_toIdleForce * deltaTime;
        m_rigidBody->applyCentralForce(-m_verticalVelocity * force);
    }

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
        transform.setOrigin(btVector3(pos.getX(), surface, pos.getZ()));
        m_rigidBody->setWorldTransform(transform);
        m_rigidBody->setLinearVelocity(btVector3(m_velocity.getX(), 0, m_velocity.getZ()));
    }

    // remove sliding caused by 0 friction
    if (!m_jumping && !m_falling && m_speed > 0.0f)
    {
        btVector3 moveDir(m_moveDir.x, m_moveDir.y, m_moveDir.z);
        btScalar moveVelocity = m_velocity.dot(moveDir);
        btVector3 velocity = moveDir * moveVelocity;
        // Save horizontal velocity
        velocity.setY(m_velocity.getY());
        m_rigidBody->setLinearVelocity(velocity);
    }
}

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t)
{
    return x * (1.f - t) + y * t;
}

float lerp(float x, float y, float t)
{
    return x * (1.f - t) + y * t;
}
