#include "character_controller.h"

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
        float force = m_rigidBody->getMass() * m_jumpForce;
        m_rigidBody->applyCentralImpulse(btVector3(0, force, 0));
    }

    // max speed check
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
        float force = m_rigidBody->getMass() * m_moveForce;
        glm::vec3 frontXZ = glm::normalize(glm::vec3(m_followCamera->front.x, 0.0f, m_followCamera->front.z));
        glm::vec3 front = frontXZ * frontForce;
        moveVec += front;
        forceVec += btVector3(front.x * force, 0.0f, front.z * force);
    }

    if (rightForce != 0.0f && !m_falling)
    {
        move = true;
        float force = m_rigidBody->getMass() * m_moveForce;
        glm::vec3 rightXZ = glm::normalize(glm::vec3(m_followCamera->right.x, 0.0f, m_followCamera->right.z));
        glm::vec3 right = rightXZ * rightForce;
        moveVec += right;
        forceVec += btVector3(right.x * force , 0.0f, right.z * force );
    }

    // TODO: prevent jump when move ends

    m_moving = move;

    if (m_moving)
    {
        // TODO: smooth transition of m_moveDir
        m_moveDir = glm::normalize(moveVec);
        m_rigidBody->setActivationState(1);
        m_rigidBody->applyCentralForce(forceVec);
    }

    // slow down and stop
    if (m_verticalSpeed > 0 && !m_moving && !m_jumping && !m_falling)
    {
        float force = m_rigidBody->getMass() * m_toIdleForce;
        m_rigidBody->applyCentralForce(-m_verticalVelocity * force);
    }

    // snap to ground
    if (m_moving && !m_jumping && !m_falling)
    {
        // float force = m_rigidBody->getMass() * m_toIdleForceHoriz;
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
}
