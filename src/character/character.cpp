#include "character.h"

Character::Character(TaskManager *taskManager, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : m_taskManager(taskManager),
      m_resourceManager(resourceManager),
      m_physicsWorld(physicsWorld),
      m_followCamera(followCamera),
      m_firing(false)
{
    init();
}

void Character::init()
{
    // Animation
    m_model = m_resourceManager->getModel("../src/assets/gltf/swat.glb");
    Animation *animation0 = new Animation("idle", m_model);
    Animation *animation1 = new Animation("walking-forward", m_model);
    Animation *animation2 = new Animation("left", m_model, true);
    Animation *animation3 = new Animation("right", m_model, true);
    Animation *animation4 = new Animation("running-forward", m_model);
    Animation *animation5 = new Animation("walking-left", m_model);
    Animation *animation6 = new Animation("walking-right", m_model);
    Animation *animation7 = new Animation("walking-back", m_model);
    Animation *animation8 = new Animation("running-left", m_model);
    Animation *animation9 = new Animation("running-right", m_model);
    Animation *animation10 = new Animation("running-back", m_model);
    Animation *animation11 = new Animation("walking-back-left", m_model);
    Animation *animation12 = new Animation("walking-back-right", m_model);
    Animation *animation13 = new Animation("running-back-left", m_model);
    Animation *animation14 = new Animation("running-back-right", m_model);
    Animation *animation15 = new Animation("pistol-aim-1", m_model, true);
    // TODO: create empty at runtime?
    Animation *animationRagdoll = new Animation("pose", m_model, true);
    Animation *animation17 = new Animation("firing", m_model, true);
    Animation *animation18 = new Animation("enter-car-5", m_model);
    Animation *animation19 = new Animation("exit-car-2", m_model);

    // TODO: inside Model
    std::vector<Animation *> animations;
    animations.push_back(animation0);
    animations.push_back(animation1);
    animations.push_back(animation2);
    animations.push_back(animation3);
    animations.push_back(animation4);
    animations.push_back(animation5);
    animations.push_back(animation6);
    animations.push_back(animation7);
    animations.push_back(animation8);
    animations.push_back(animation9);
    animations.push_back(animation10);
    animations.push_back(animation11);
    animations.push_back(animation12);
    animations.push_back(animation13);
    animations.push_back(animation14);
    animations.push_back(animation15);
    animations.push_back(animationRagdoll);
    animations.push_back(animation17);
    animations.push_back(animation18);
    animations.push_back(animation19);

    // TODO: setup multiple animators from same Model
    m_animator = new Animator(animations);
    m_animator->m_startOffset = 33.333f;

    // state
    // TODO: increase accessibility - name or pointer to Animation
    Anim anim;
    anim.index = 0; // idle
    anim.blendFactor = 1.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 1; // forward
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 7; // backward
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 11; // backward-left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 12; // backward-right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 5; // left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 6; // right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 4; // run-forward
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 10; // run-back
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 8; // run-left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 9; // run-right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 13; // run-back-left
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);
    anim.index = 14; // run-back-right
    anim.blendFactor = 0.0;
    m_animator->m_state.animations.push_back(anim);

    // set blend mask for turn-right and turn-left
    std::unordered_map<std::string, float> blendMask;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;

    animation2->setBlendMask(blendMask, 0.f);
    animation3->setBlendMask(blendMask, 0.f);
    // TODO: ragdoll mask for ragdoll hands - default position?

    // TODO: fix animation - no arms
    blendMask.clear();
    blendMask["mixamorig:Hips"] = 1.0f;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;
    blendMask["mixamorig:LeftShoulder"] = 1.0f;
    blendMask["mixamorig:RightShoulder"] = 1.0f;
    blendMask["mixamorig:RightUpLeg"] = 1.0f;
    blendMask["mixamorig:RightLeg"] = 1.0f;
    blendMask["mixamorig:RightFoot"] = 1.0f;
    blendMask["mixamorig:RightToeBase"] = 1.0f;
    blendMask["mixamorig:LeftUpLeg"] = 1.0f;
    blendMask["mixamorig:LeftLeg"] = 1.0f;
    blendMask["mixamorig:LeftFoot"] = 1.0f;
    blendMask["mixamorig:LeftToeBase"] = 1.0f;

    animation11->setBlendMask(blendMask, 0.f);
    animation12->setBlendMask(blendMask, 0.f);
    animation13->setBlendMask(blendMask, 0.f);
    animation14->setBlendMask(blendMask, 0.f);

    // firing
    blendMask.clear();
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:RightShoulder"] = 1.0f;
    blendMask["mixamorig:RightArm"] = 1.0f;
    blendMask["mixamorig:RightForeArm"] = 1.0f;
    blendMask["mixamorig:RightHand"] = 1.0f;

    animation17->setBlendMask(blendMask, 0.f);

    // turn-left pose
    AnimPose animPose;
    animPose.index = 2;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // turn-right pose
    animPose.index = 3;
    m_animator->m_state.poses.push_back(animPose);
    // pistol-aim
    animPose.index = 15;
    animPose.blendFactor = 1.0f;
    m_animator->m_state.poses.push_back(animPose);
    // ragdoll
    animPose.index = 16;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // firing
    animPose.index = 17;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // enter-car
    animPose.index = 18;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);
    // exit-car
    animPose.index = 19;
    animPose.blendFactor = 0.0f;
    m_animator->m_state.poses.push_back(animPose);

    m_walkStepFreq = 1.f / m_animator->m_animations[1]->m_Duration;
    m_runStepFreq = 1.f / m_animator->m_animations[4]->m_Duration;

    // TODO:
    for (int i = 0; i < 13; i++)
        m_blendTargets[i] = 0.0f;

    // Character
    m_rigidbody = m_physicsWorld->createCapsule(10.0f, 1.0f, 0.5f, 2.0f, BulletGLM::getBulletVec3(m_position));
    m_rigidbody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    m_rigidbody->setDamping(0.9f, 0.9f);
    m_rigidbody->setFriction(0.0f);
    m_rigidbody->setGravity(btVector3(0, -20.0f, 0));

    m_controller = new CharacterController(m_physicsWorld->m_dynamicsWorld, m_rigidbody, m_followCamera);
    m_ragdoll = new Ragdoll(m_physicsWorld, animationRagdoll, BulletGLM::getBulletVec3(m_position), 2.0f);
}

Character::~Character()
{
    delete m_controller;
    for (int i = 0; i < m_animator->m_animations.size(); i++)
        delete m_animator->m_animations[i];
    m_animator->m_animations.clear();
    delete m_animator;
    delete m_ragdoll;
}

void Character::update(float deltaTime)
{
    // update character
    if (!m_ragdollActive)
        m_controller->update(deltaTime);

    // update ragdoll
    m_ragdoll->update(deltaTime);
    // checkPhysicsStateChange();
    if (m_ragdollActive)
        m_ragdoll->syncToAnimation(m_position);

    // update animation
    m_animator->update(deltaTime);

    // update ragdoll blend
    AnimPose &ragdolPose = getRagdolPose();
    ragdolPose.blendFactor += deltaTime * m_stateChangeSpeed * (m_ragdollActive ? 1.f : -1.f);
    ragdolPose.blendFactor = std::max(0.0f, std::min(ragdolPose.blendFactor, 1.0f));

    // update pistol-aim blend
    m_aimBlend += deltaTime * m_aimStateChangeSpeed * (m_controller->m_aimLocked ? 1.f : -1.f);
    m_aimBlend = std::max(0.0f, std::min(m_aimBlend, 1.0f));
    updateAimPoseBlendMask(m_aimBlend);

    // update firing blend
    AnimPose &firingPose = getFiringPose();
    firingPose.blendFactor += deltaTime * m_firingStateChangeSpeed * (m_firing ? 1.f : -1.f);
    firingPose.blendFactor = std::max(0.0f, std::min(firingPose.blendFactor, 1.0f));

    // sync animator with controller
    if (m_rigidbody && m_rigidbody->getMotionState() && !m_ragdollActive)
    {
        if (m_syncPositionFromPhysics)
        {
            btTransform trans;
            m_rigidbody->getMotionState()->getWorldTransform(trans);
            m_position = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            m_position -= glm::vec3(0, m_controller->m_halfHeight, 0);
        }
        m_rotation.y = glm::atan(m_controller->m_lookDir.x, m_controller->m_lookDir.z);

        float front = 0.f;
        float back = 0.f;
        float right = 0.f;
        float left = 0.f;
        float backRight = 0.f;
        float backLeft = 0.f;
        float moveAngle = m_controller->m_signedMoveAngle;

        if (moveAngle >= 0.f && moveAngle < M_PI_2) // 1
        {
            front = (M_PI_2 - moveAngle) / M_PI_2;
            right = moveAngle / M_PI_2;
        }
        else if (moveAngle >= M_PI_2 && moveAngle < M_PI_2 + M_PI_2 / 2.f) // 2.1
        {
            float angle = moveAngle - M_PI_2;
            backRight = angle / (M_PI_2 / 2.f);
            right = 1.f - backRight;
        }
        else if (moveAngle >= M_PI_2 + M_PI_2 / 2.f && moveAngle <= M_PI) // 2.2
        {
            float angle = moveAngle - (M_PI_2 + M_PI_2 / 2.f);
            back = angle / (M_PI_2 / 2.f);
            backRight = 1.f - back;
        }
        else if (moveAngle >= -M_PI && moveAngle < -(M_PI_2 + M_PI_2 / 2.f)) // 3.1
        {
            float angle = -moveAngle - (M_PI_2 + M_PI_2 / 2.f);
            back = angle / (M_PI_2 / 2.f);
            backLeft = 1.f - back;
        }
        else if (moveAngle >= -(M_PI_2 + M_PI_2 / 2.f) && moveAngle < -M_PI_2) // 3.2
        {
            float angle = -moveAngle - M_PI_2;
            backLeft = angle / (M_PI_2 / 2.f);
            left = 1.f - backLeft;
        }
        else if (moveAngle >= -M_PI_2 && moveAngle < 0.f) // 4
        {
            front = (M_PI_2 + moveAngle) / M_PI_2;
            left = -moveAngle / M_PI_2;
        }

        m_blendTargets[0] = 0.f; // idle
        m_blendTargets[1] = front;
        m_blendTargets[2] = back;
        m_blendTargets[3] = backLeft;
        m_blendTargets[4] = backRight;
        m_blendTargets[5] = left;
        m_blendTargets[6] = right;
        m_blendTargets[7] = 0.f;  // run-forward
        m_blendTargets[8] = 0.f;  // run-back
        m_blendTargets[9] = 0.f;  // run-left
        m_blendTargets[10] = 0.f; // run-right
        m_blendTargets[11] = 0.f; // run-back-left
        m_blendTargets[12] = 0.f; // run-back-right

        m_runFactor = 0.f;
        m_idleFactor = m_animator->m_state.animations[0].blendFactor;

        float maxWalkSpeed = m_controller->m_maxWalkRelative + m_controller->m_walkToRunAnimThreshold;
        if (m_controller->m_verticalSpeed > maxWalkSpeed)
        {
            float runnningGap = m_controller->m_maxRunRelative - maxWalkSpeed;
            float runningLevel = m_controller->m_verticalSpeed - maxWalkSpeed;
            m_runFactor = CommonUtil::snappedClamp(runningLevel / runnningGap, 0.0f, 1.0f, 0.08f);
            float clamped = m_runFactor;

            m_blendTargets[1] *= (1.f - clamped);
            m_blendTargets[2] *= (1.f - clamped);
            m_blendTargets[3] *= (1.f - clamped);
            m_blendTargets[4] *= (1.f - clamped);
            m_blendTargets[5] *= (1.f - clamped);
            m_blendTargets[6] *= (1.f - clamped);
            m_blendTargets[7] = front * clamped;
            m_blendTargets[8] = back * clamped;
            m_blendTargets[9] = left * clamped;
            m_blendTargets[10] = right * clamped;
            m_blendTargets[11] = backLeft * clamped;
            m_blendTargets[12] = backRight * clamped;
        }
        else
        {
            float clamped = CommonUtil::snappedClamp(m_controller->m_verticalSpeed / m_controller->m_maxWalkRelative, 0.0f, 1.0f, 0.0f);

            m_blendTargets[0] = (1.f - clamped);
            m_blendTargets[1] *= clamped;
            m_blendTargets[2] *= clamped;
            m_blendTargets[3] *= clamped;
            m_blendTargets[4] *= clamped;
            m_blendTargets[5] *= clamped;
            m_blendTargets[6] *= clamped;
        }

        syncFootstepFrequency();

        // TODO: fix - wrong blend while locked
        // full body rotating
        if (m_controller->m_dotFront <= 0.f && m_controller->m_turnFactor > 0)
        {
            float turn = m_controller->m_turnFactor / m_controller->m_turnAnimMaxFactor;
            m_blendTargets[4] = turn * m_controller->m_turnAnimMult;

            m_blendTargets[3] *= (1.f - turn);
            m_blendTargets[1] *= (1.f - turn);
            m_blendTargets[2] *= (1.f - turn);
            m_blendTargets[5] *= (1.f - turn);
            m_blendTargets[6] *= (1.f - turn);
        }
        else if (m_controller->m_dotFront <= 0.f && m_controller->m_turnFactor < 0)
        {
            float turn = -m_controller->m_turnFactor / m_controller->m_turnAnimMaxFactor;
            m_blendTargets[3] = turn * m_controller->m_turnAnimMult;

            m_blendTargets[4] *= (1.f - turn);
            m_blendTargets[1] *= (1.f - turn);
            m_blendTargets[2] *= (1.f - turn);
            m_blendTargets[5] *= (1.f - turn);
            m_blendTargets[6] *= (1.f - turn);
        }

        float &animL = m_blendTargetsPose[0];
        float &animR = m_blendTargetsPose[1];
        animL = std::max(0.0f, std::min(-m_controller->m_turnFactor, 1.0f));
        animR = std::max(0.0f, std::min(m_controller->m_turnFactor, 1.0f));

        // adaptive turn for pistol-aim
        if (m_controller->m_aimLocked)
        {
            float &leftBlend = m_animator->m_state.animations[5].blendFactor;
            float &rightBlend = m_animator->m_state.animations[6].blendFactor;

            float leftB = leftBlend + m_leftForward;
            float rightB = rightBlend + m_rightForward;

            if (leftB > 0.f)
                animR = std::max(0.0f, std::min(leftB / m_leftBlendEdge, 1.0f));
            if (rightB > 0.f)
                animL = std::max(0.0f, std::min(rightB / m_rightBlendEdge, 1.0f));
        }

        m_prevRunFactor = m_runFactor;
        m_prevIdleFactor = m_idleFactor;

        interpolateBlendTargets();
    }

    updateModelMatrix();
}

void Character::updateModelMatrix()
{
    glm::mat4 model = glm::mat4(1.0f);

    if (m_passengerInfo.state == PassengerState::inside ||
        m_passengerInfo.state == PassengerState::exiting)
    {
        CarController *car = m_passengerInfo.car;
        Vehicle *vehicle = car->m_vehicle;
        model = glm::translate(vehicle->m_chassisModel, car->m_animDoorOffset);
        model = glm::rotate(model, car->m_rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, car->m_rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, car->m_rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(m_scale));
    }
    else
    {
        model = glm::translate(model, m_position);
        model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, m_rotation.y * (1.0f - getRagdolPose().blendFactor), glm::vec3(0, 1, 0));
        model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(m_scale));
    }

    m_modelMatrix = model;
}

void Character::interpolateBlendTargets()
{
    for (int i = 0; i < 13; i++)
        m_animator->m_state.animations[i].blendFactor = CommonUtil::lerp(m_animator->m_state.animations[i].blendFactor, m_blendTargets[i], m_blendSpeed);
    for (int i = 0; i < 2; i++)
        m_animator->m_state.poses[i].blendFactor = CommonUtil::lerp(m_animator->m_state.poses[i].blendFactor, m_blendTargetsPose[i], m_blendSpeed);
}

void Character::syncFootstepFrequency()
{
    if (m_runFactor > 0.f && m_runFactor < 1.f)
    {
        m_walkPerc = m_animator->m_timers[1] / m_animator->m_animations[1]->m_Duration;
        m_runPerc = m_animator->m_timers[4] / m_animator->m_animations[4]->m_Duration;

        m_walkAnimSpeed = (m_walkStepFreq + (m_runStepFreq - m_walkStepFreq) * m_runFactor) / m_walkStepFreq;
        m_runAnimSpeed = (m_walkStepFreq + (m_runStepFreq - m_walkStepFreq) * m_runFactor) / m_runStepFreq;

        setWalkPlaybackSpeed(m_walkAnimSpeed);
        setRunPlaybackSpeed(m_runAnimSpeed);

        // refresh to fix error gap
        float timerGap = std::fabs(m_walkPerc - m_runPerc);
        bool timerOff = timerGap > 0.01f;

        // if (timerOff)
        //     std::cout << "timerOff: timerGap: " << timerGap << std::endl;

        if (m_prevRunFactor == 0.f || (timerOff && m_runFactor < 0.5f))
        {
            float runTimer = m_walkPerc * m_animator->m_animations[4]->m_Duration;
            setRunTimer(runTimer);
        }
        else if (m_prevRunFactor == 1.f || timerOff)
        {
            float walkTimer = m_runPerc * m_animator->m_animations[1]->m_Duration;
            setWalkTimer(walkTimer);
        }
    }
    else
    {
        if (m_runFactor == 0.f)
            setWalkPlaybackSpeed(1.f);
        else if (m_runFactor == 1.f)
            setRunPlaybackSpeed(1.f);
    }

    // return to idle
    float gap = m_idleFactor - m_prevIdleFactor;
    if (m_idleFactor != 1.f && gap > 0.01f) // stopping
    {
        float nextTime;
        float fullTime = m_animator->m_animations[1]->m_Duration;
        float halfTime = fullTime / 2.f;

        float time = m_animator->m_timers[1];

        // TODO: better sync
        if (time > halfTime)
            nextTime = fullTime;
        else
            nextTime = halfTime;

        m_animator->m_timers[1] = CommonUtil::lerp(m_animator->m_timers[1], nextTime, m_stopBlendSpeed);
        m_animator->m_timers[7] = CommonUtil::lerp(m_animator->m_timers[7], nextTime, m_stopBlendSpeed);
        m_animator->m_timers[11] = CommonUtil::lerp(m_animator->m_timers[11], nextTime, m_stopBlendSpeed);
        m_animator->m_timers[12] = CommonUtil::lerp(m_animator->m_timers[12], nextTime, m_stopBlendSpeed);
        m_animator->m_timers[5] = CommonUtil::lerp(m_animator->m_timers[5], nextTime, m_stopBlendSpeed);
        m_animator->m_timers[6] = CommonUtil::lerp(m_animator->m_timers[6], nextTime, m_stopBlendSpeed);

        // TODO: stop character at stop anim position
    }
}

void Character::setWalkPlaybackSpeed(float animSpeed)
{
    m_animator->m_animations[1]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[7]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[11]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[12]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[5]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[6]->m_playbackSpeed = animSpeed;
}

void Character::setRunPlaybackSpeed(float animSpeed)
{
    m_animator->m_animations[4]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[10]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[8]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[9]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[13]->m_playbackSpeed = animSpeed;
    m_animator->m_animations[14]->m_playbackSpeed = animSpeed;
}

void Character::setWalkTimer(float time)
{
    m_animator->m_timers[1] = time;
    m_animator->m_timers[7] = time;
    m_animator->m_timers[11] = time;
    m_animator->m_timers[12] = time;
    m_animator->m_timers[5] = time;
    m_animator->m_timers[6] = time;
}

void Character::setRunTimer(float time)
{
    m_animator->m_timers[4] = time;
    m_animator->m_timers[10] = time;
    m_animator->m_timers[8] = time;
    m_animator->m_timers[9] = time;
    m_animator->m_timers[13] = time;
    m_animator->m_timers[14] = time;
}

void Character::activateRagdoll(glm::vec3 impulse)
{
    inactivateCollider();
    btVector3 modelPos = BulletGLM::getBulletVec3(m_position);
    m_ragdoll->resetTransforms(modelPos, m_rotation.y);
    m_ragdoll->unFreezeBodies();
    m_ragdoll->changeState(RagdollState::fetal);
    m_ragdollActive = true;

    btRigidBody *pelvis = m_ragdoll->m_bodies[BODYPART_PELVIS];
    btRigidBody *spine = m_ragdoll->m_bodies[BODYPART_SPINE];

    pelvis->applyCentralImpulse(BulletGLM::getBulletVec3(impulse * 0.4f));
    spine->applyCentralImpulse(BulletGLM::getBulletVec3(impulse * 0.6f));
}

void Character::resetRagdoll()
{
    btVector3 modelPos = BulletGLM::getBulletVec3(m_position + glm::vec3(0.f, 10.f, 0.f));
    m_ragdoll->resetTransforms(modelPos, m_rotation.y);
    m_ragdoll->freezeBodies();
    m_ragdoll->changeState(RagdollState::loose);
    activateCollider();
    m_ragdollActive = false;
}

void Character::activateCollider()
{
    m_rigidbody->setLinearFactor(btVector3(1.0f, 1.0f, 1.0f));
    m_rigidbody->setActivationState(1);
    m_rigidbody->setCollisionFlags(m_rigidbody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void Character::inactivateCollider()
{
    m_rigidbody->setLinearFactor(btVector3(0.0f, 0.0f, 0.0f));
    m_rigidbody->setActivationState(0);
    m_rigidbody->setCollisionFlags(m_rigidbody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

void Character::checkPhysicsStateChange()
{
    btVector3 totalForce = m_rigidbody->getTotalForce();
    float force = totalForce.length();
    if (force > m_ragdolActivateThreshold)
    {
        activateRagdoll(-BulletGLM::getGLMVec3(totalForce) * m_ragdolActivateFactor);
    }
}

AnimPose &Character::getRagdolPose()
{
    return m_animator->m_state.poses[3];
}

AnimPose &Character::getAimPose()
{
    return m_animator->m_state.poses[2];
}

AnimPose &Character::getFiringPose()
{
    return m_animator->m_state.poses[4];
}

AnimPose &Character::getEnterCarAnim()
{
    return m_animator->m_state.poses[5];
}

AnimPose &Character::getExitCarAnim()
{
    return m_animator->m_state.poses[6];
}

// TODO: single run in a thread
void Character::updateAimPoseBlendMask(float blendFactor)
{
    // early exit
    Bone *bone = m_animator->m_animations[15]->getBone("mixamorig:Head");
    if (bone && bone->m_blendFactor == blendFactor)
        return;

    std::unordered_map<std::string, float> blendMask;

    glm::vec2 p0(0, 0);
    glm::vec2 p1(0.5, 1);
    glm::vec2 p2(0.15, 1);
    glm::vec2 p3(1, 1);
    float cubicBlend = CommonUtil::cubicBezier(p0, p1, p2, p3, blendFactor).y;

    blendMask["mixamorig:Head"] = cubicBlend;
    blendMask["mixamorig:Neck"] = cubicBlend;
    blendMask["mixamorig:Spine2"] = cubicBlend;
    blendMask["mixamorig:RightShoulder"] = cubicBlend;
    blendMask["mixamorig:RightArm"] = cubicBlend;
    blendMask["mixamorig:RightForeArm"] = cubicBlend;
    blendMask["mixamorig:RightHand"] = cubicBlend;

    // always on
    blendMask["mixamorig:RightHandThumb1"] = 1.0f;
    blendMask["mixamorig:RightHandThumb2"] = 1.0f;
    blendMask["mixamorig:RightHandThumb3"] = 1.0f;
    blendMask["mixamorig:RightHandThumb4"] = 1.0f;
    blendMask["mixamorig:RightHandIndex1"] = 1.0f;
    blendMask["mixamorig:RightHandIndex2"] = 1.0f;
    blendMask["mixamorig:RightHandIndex3"] = 1.0f;
    blendMask["mixamorig:RightHandIndex4"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle1"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle2"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle3"] = 1.0f;
    blendMask["mixamorig:RightHandMiddle4"] = 1.0f;
    blendMask["mixamorig:RightHandRing1"] = 1.0f;
    blendMask["mixamorig:RightHandRing2"] = 1.0f;
    blendMask["mixamorig:RightHandRing3"] = 1.0f;
    blendMask["mixamorig:RightHandRing4"] = 1.0f;
    blendMask["mixamorig:RightHandPinky1"] = 1.0f;
    blendMask["mixamorig:RightHandPinky2"] = 1.0f;
    blendMask["mixamorig:RightHandPinky3"] = 1.0f;
    blendMask["mixamorig:RightHandPinky4"] = 1.0f;

    m_animator->m_animations[15]->setBlendMask(blendMask, 0.0f);
}

// TODO: move
class MyContactResultCallback : public btCollisionWorld::ContactResultCallback
{
public:
    CarController *m_controller = nullptr;

    virtual btScalar addSingleResult(btManifoldPoint &cp,
                                     const btCollisionObjectWrapper *colObj0Wrap,
                                     int partId0,
                                     int index0,
                                     const btCollisionObjectWrapper *colObj1Wrap,
                                     int partId1,
                                     int index1) override
    {
        void *userPointer0 = colObj0Wrap->getCollisionObject()->getUserPointer();
        void *userPointer1 = colObj1Wrap->getCollisionObject()->getUserPointer();

        CarController *controller = dynamic_cast<CarController *>(static_cast<CarController *>(userPointer0));
        if (!controller)
            return 1.0f;

        m_controller = controller;

        return 0.0f;
    }
};

void Character::interruptExitCar()
{
    auto callback = [&](CharacterTask *task)
    {
        if (ExitCar *exitCar = dynamic_cast<ExitCar *>(static_cast<ExitCar *>(task)))
            return exitCar->m_character == this;

        return false;
    };
    std::vector<CharacterTask *> pointers = m_taskManager->getTaskPointers(callback);

    for (CharacterTask *task : pointers)
        task->interrupt();
}

// TODO: can not interrupt if the entering process is beyond some level
void Character::cancelEnterCar()
{
    // TODO: can not called from playable_character - check still?

    auto callback = [&](CharacterTask *task)
    {
        if (FollowPath *followPath = dynamic_cast<FollowPath *>(static_cast<FollowPath *>(task)))
            return followPath->m_source == this;
        else if (EnterCar *enterCar = dynamic_cast<EnterCar *>(static_cast<EnterCar *>(task)))
            return enterCar->m_character == this;

        return false;
    };
    std::vector<CharacterTask *> pointers = m_taskManager->getTaskPointers(callback);

    // can't cancel if EnterCar already started
    if (pointers.size() == 1)
        return;

    for (CharacterTask *task : pointers)
        task->interrupt();
}

// TODO: move
// TODO: run in a thread
void Character::enterNearestCar()
{
    // check if there is any car in a distance
    btScalar radius = 30.0f;
    btSphereShape queryShape(radius);

    btTransform queryTransform;
    queryTransform.setIdentity();
    btVector3 position = BulletGLM::getBulletVec3(m_position);
    queryTransform.setOrigin(position);

    btCollisionObject queryObject;
    queryObject.setCollisionShape(&queryShape);
    queryObject.setWorldTransform(queryTransform);

    btCollisionWorld *collisionWorld = m_physicsWorld->m_dynamicsWorld->getCollisionWorld();
    MyContactResultCallback callback;
    collisionWorld->contactTest(&queryObject, callback);

    if (!callback.m_controller)
    {
        // std::cout << "enterNearestCar: no car found" << std::endl;
        return;
    }

    m_passengerInfo.state = PassengerState::entering;

    // std::cout << "enterNearestCar: car found" << std::endl;

    // find far distance
    float carWidth = callback.m_controller->m_safeSize.x;
    float carLenght = callback.m_controller->m_safeSize.y;
    glm::vec2 doorOffset = callback.m_controller->m_doorOffset;

    std::vector<glm::vec3> corners;
    corners.push_back(CommonUtil::positionFromModel(callback.m_controller->translateOffset(glm::vec3(carWidth, 0.f, carLenght))));
    corners.push_back(CommonUtil::positionFromModel(callback.m_controller->translateOffset(glm::vec3(-carWidth, 0.f, carLenght))));
    corners.push_back(CommonUtil::positionFromModel(callback.m_controller->translateOffset(glm::vec3(-carWidth, 0.f, -carLenght))));
    corners.push_back(CommonUtil::positionFromModel(callback.m_controller->translateOffset(glm::vec3(carWidth, 0.f, -carLenght))));
    glm::vec3 frontDoor = CommonUtil::positionFromModel(callback.m_controller->translateOffset(glm::vec3(doorOffset.x, 0.f, doorOffset.y)));

    float farthest = 0.f;
    for (glm::vec3 &corner : corners)
    {
        float distance = glm::distance(corner, m_position);
        if (distance > farthest)
            farthest = distance;
    }

    // create jsp
    int size = (ceil(farthest) + 2.f) * 2;

    Vec2i dim(size, size);
    JPS::Tmap data;
    data.resize(dim.x() * dim.y(), 0);

    glm::vec2 start((float)size / 2, (float)size / 2);

    // TODO: other colliders
    // insert car into map
    std::vector<std::vector<glm::vec2>> points;
    points.push_back(CommonUtil::getLinePoints(glm::vec2(corners[0].x, corners[0].z), glm::vec2(corners[1].x, corners[1].z)));
    points.push_back(CommonUtil::getLinePoints(glm::vec2(corners[1].x, corners[1].z), glm::vec2(corners[2].x, corners[2].z)));
    points.push_back(CommonUtil::getLinePoints(glm::vec2(corners[2].x, corners[2].z), glm::vec2(corners[3].x, corners[3].z)));
    points.push_back(CommonUtil::getLinePoints(glm::vec2(corners[3].x, corners[3].z), glm::vec2(corners[0].x, corners[0].z)));

    for (std::vector<glm::vec2> &pointList : points)
    {
        for (glm::vec2 &point : pointList)
        {
            glm::vec2 offset = point - glm::vec2(m_position.x, m_position.z);
            glm::vec2 p = start + offset;
            data[size * round(p.y) + round(p.x)] = 100;
        }
    }

    // found the goal point
    // find path
    std::shared_ptr<JPS::OccMapUtil> map_util = std::make_shared<JPS::OccMapUtil>();
    map_util->setMap(Vec2f(0, 0), dim, data, 1);

    glm::vec2 offset = glm::vec2(frontDoor.x, frontDoor.z) - glm::vec2(m_position.x, m_position.z);
    glm::vec2 offset2 = start + offset;
    const Vec2f goal(offset2.x, offset2.y);

    JPSPlanner2D planner(false);
    planner.setMapUtil(map_util);
    planner.updateMap();                                               // Set map, must be called before plan
    bool valid = planner.plan(Vec2f(start.x, start.y), goal, 1, true); // Plan from start to goal with heuristic weight 1, using JPS
    vec_Vec2f path = planner.getRawPath();

    // TODO: optionally enable
    // if (dmEnabled)
    {
        DMPlanner2D dmp(false);
        dmp.setPotentialRadius(Vec2f(2.f, 2.f));
        dmp.setSearchRadius(Vec2f(2.f, 2.f));
        dmp.setMap(map_util, Vec2f(start.x, start.y));
        bool valid = dmp.computePath(Vec2f(start.x, start.y), goal, path);
        if (valid)
            path = dmp.getRawPath();
    }

    PathResult result;
    result.dim = dim;
    result.data = data;
    result.start = Vec2f(start.x, start.y);
    result.startWorld = Vec2f(m_position.x, m_position.z);
    result.goal = goal;
    result.path = path;
    result.empty = false;

    // std::cout << "valid: " << valid << ", size: " << path.size() << std::endl;
    // std::cout << "dim: (" << dim.x() << ", " << dim.y() << ")" << std::endl;
    // std::cout << "start: (" << start.x << ", " << start.y << ")" << std::endl;
    // std::cout << "goal: (" << goal.x() << ", " << goal.y() << ")" << std::endl;

    m_lastCarEnterPath = result;

    // create task
    std::vector<glm::vec3> resultPath;
    for (Vec2f &point : path)
    {
        glm::vec3 offset = glm::vec3(point.x(), 0.f, point.y()) - glm::vec3(start.x, 0, start.y);
        resultPath.push_back(m_position + offset);
    }
    // goal point
    glm::vec3 offset3 = glm::vec3(goal.x(), 0.f, goal.y()) - glm::vec3(start.x, 0, start.y);
    resultPath.push_back(m_position + offset3);

    glm::vec3 doorDir = glm::normalize(corners[1] - corners[0]);
    doorDir = glm::normalize(glm::vec3(doorDir.x, 0, doorDir.z));
    FollowPath *followPath = new FollowPath(this, resultPath, doorDir);
    EnterCar *enterCar = new EnterCar(this, callback.m_controller);

    std::stack<CharacterTask *> taskStack;
    taskStack.push(enterCar);
    taskStack.push(followPath);
    cancelEnterCar();
    m_taskManager->addTaskStack(taskStack);
}

void Character::exitFromCar()
{
    if (m_passengerInfo.state != PassengerState::inside)
    {
        std::cout << "exitFromCar: not inside a car" << std::endl;
        return;
    }

    m_passengerInfo.state = PassengerState::exiting;
    m_passengerInfo.exitRequested = true;

    ExitCar *exitCar = new ExitCar(this, m_passengerInfo.car);

    m_taskManager->addTask(exitCar);
}
