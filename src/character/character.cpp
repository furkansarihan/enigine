#include "character.h"

Character::Character(RenderManager *renderManager, ResourceManager *resourceManager, PhysicsWorld *physicsWorld, Camera *followCamera)
    : m_renderManager(renderManager),
      m_resourceManager(resourceManager),
      m_physicsWorld(physicsWorld),
      m_followCamera(followCamera),
      m_firing(false),
      m_headRotOffset(glm::quat(1.f, 0.f, 0.f, 0.f)),
      m_clampedHeadRot(glm::quat(1.f, 0.f, 0.f, 0.f)),
      m_controlCharacter(false),
      m_followCharacter(false),
      m_headFollow(false)
{
    init();
}

void Character::init()
{
    // Animation
    m_model = m_resourceManager->getModel("assets/character/mixamo-y-1.glb");

    Animation *animIdle = new Animation("idle", m_model);

    Animation *animWalkForward = new Animation("walking-forward", m_model);
    Animation *animWalkBack = new Animation("walking-back", m_model);
    Animation *animWalkLeft = new Animation("walking-left", m_model);
    Animation *animWalkRight = new Animation("walking-right", m_model);
    Animation *animWalkBackLeft = new Animation("walking-back-left", m_model);
    Animation *animWalkBackRight = new Animation("walking-back-right", m_model);

    Animation *animRunForward = new Animation("running-forward", m_model);
    Animation *animRunBack = new Animation("running-back", m_model);
    Animation *animRunLeft = new Animation("running-left", m_model);
    Animation *animRunRight = new Animation("running-right", m_model);
    Animation *animRunBackLeft = new Animation("running-back-left", m_model);
    Animation *animRunBackRight = new Animation("running-back-right", m_model);

    // TODO: create empty at runtime?
    Animation *animRagdoll = new Animation("pose", m_model);
    Animation *animHeadFollow = new Animation("pose", m_model);

    Animation *animPistolAim = new Animation("pistol-aim-1", m_model);
    Animation *animFiring = new Animation("firing", m_model);
    Animation *animLeanLeft = new Animation("left", m_model);
    Animation *animLeanRight = new Animation("right", m_model);

    Animation *animEnterCar = new Animation("enter-car-7", m_model);
    Animation *animExitCar = new Animation("exit-car-6", m_model);
    Animation *animJumpCar = new Animation("jump-car-5", m_model);
    Animation *animTurn180 = new Animation("turn-180", m_model);

    // TODO: inside Model
    std::vector<Animation *> animations = {
        // movement
        animIdle,
        animWalkForward,
        animWalkBack,
        animWalkLeft,
        animWalkRight,
        animWalkBackLeft,
        animWalkBackRight,
        animRunForward,
        animRunBack,
        animRunLeft,
        animRunRight,
        animRunBackLeft,
        animRunBackRight,
        // empty
        animRagdoll,
        animHeadFollow,
        // pose
        animPistolAim,
        animFiring,
        animLeanLeft,
        animLeanRight,
        // actions
        animEnterCar,
        animExitCar,
        animJumpCar,
    };

    // TODO: setup multiple animators from same Model
    m_animator = new Animator(animations);
    m_animator->m_startOffset = 33.333f;

    // states
    m_idle = m_animator->addStateAnimation(animIdle);
    m_idle->m_blendFactor = 1.0f; // TODO: ?

    m_walkCircle.m_forward = m_animator->addStateAnimation(animWalkForward);
    m_walkCircle.m_back = m_animator->addStateAnimation(animWalkBack);
    m_walkCircle.m_left = m_animator->addStateAnimation(animWalkLeft);
    m_walkCircle.m_right = m_animator->addStateAnimation(animWalkRight);
    m_walkCircle.m_backLeft = m_animator->addStateAnimation(animWalkBackLeft);
    m_walkCircle.m_backRight = m_animator->addStateAnimation(animWalkBackRight);

    m_runCircle.m_forward = m_animator->addStateAnimation(animRunForward);
    m_runCircle.m_back = m_animator->addStateAnimation(animRunBack);
    m_runCircle.m_left = m_animator->addStateAnimation(animRunLeft);
    m_runCircle.m_right = m_animator->addStateAnimation(animRunRight);
    m_runCircle.m_backLeft = m_animator->addStateAnimation(animRunBackLeft);
    m_runCircle.m_backRight = m_animator->addStateAnimation(animRunBackRight);

    // blend masks
    std::unordered_map<std::string, float> blendMask;
    blendMask["mixamorig:Spine"] = 1.0f;
    blendMask["mixamorig:Spine1"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Head"] = 1.0f;

    animLeanLeft->setBlendMask(blendMask, 0.f);
    animLeanRight->setBlendMask(blendMask, 0.f);
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

    animWalkBackLeft->setBlendMask(blendMask, 0.f);
    animWalkBackRight->setBlendMask(blendMask, 0.f);
    animRunBackLeft->setBlendMask(blendMask, 0.f);
    animRunBackRight->setBlendMask(blendMask, 0.f);

    // firing
    blendMask.clear();
    blendMask["mixamorig:Neck"] = 1.0f;
    blendMask["mixamorig:Spine2"] = 1.0f;
    blendMask["mixamorig:RightShoulder"] = 1.0f;
    blendMask["mixamorig:RightArm"] = 1.0f;
    blendMask["mixamorig:RightForeArm"] = 1.0f;
    blendMask["mixamorig:RightHand"] = 1.0f;
    animFiring->setBlendMask(blendMask, 0.f);

    // head follow
    blendMask.clear();
    blendMask["mixamorig:Head"] = 1.0f;
    animHeadFollow->setBlendMask(blendMask, 0.f);

    blendMask.clear();
    blendMask["mixamorig:Hips"] = 0.f;
    animTurn180->setBlendMask(blendMask, 1.f);

    // TODO: priority variable? instead of order
    // state anim poses
    m_animPoseLeanLeft = m_animator->addPoseAnimation(animLeanLeft);
    m_animPoseLeanRight = m_animator->addPoseAnimation(animLeanRight);
    m_animPosePistolAim = m_animator->addPoseAnimation(animPistolAim);
    m_animPoseFiring = m_animator->addPoseAnimation(animFiring);
    m_animPoseEnterCar = m_animator->addPoseAnimation(animEnterCar);
    m_animPoseEnterCar = m_animator->addPoseAnimation(animExitCar);
    m_animPoseJumpCar = m_animator->addPoseAnimation(animJumpCar);
    m_animTurn180 = m_animator->addPoseAnimation(animTurn180);
    m_animPoseHeadFollow = m_animator->addPoseAnimation(animHeadFollow);
    m_animPoseRagdoll = m_animator->addPoseAnimation(animRagdoll);

    m_animPoseRagdoll->m_timerActive = false;
    m_animPoseHeadFollow->m_timerActive = false;
    m_animPosePistolAim->m_timerActive = false;
    m_animPoseFiring->m_timerActive = false;
    m_animPoseLeanLeft->m_timerActive = false;
    m_animPoseLeanRight->m_timerActive = false;

    m_walkStepFreq = 1.f / animWalkForward->m_duration;
    m_runStepFreq = 1.f / animRunForward->m_duration;

    m_moveStage.idle = 1.f;

    // Character
    m_rigidbody = m_physicsWorld->createCapsule(10.0f, 1.0f, 0.25f, 1.3f, BulletGLM::getBulletVec3(m_position));
    m_rigidbody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    m_rigidbody->setDamping(0.9f, 0.9f);
    m_rigidbody->setFriction(0.0f);
    m_rigidbody->setGravity(btVector3(0, -10.0f, 0));

    m_controller = new CharacterController(m_physicsWorld->m_dynamicsWorld, m_rigidbody, m_followCamera);
    m_ragdoll = new Ragdoll(m_physicsWorld, m_animator, animRagdoll, BulletGLM::getBulletVec3(m_position), 1.0f);
    m_ragdoll->m_modelOffset = glm::vec3(0.f, -1.0f, 0.f);

    eTransform transform;
    // TODO: blender gltf exporter bug - when root armature is directly parented to skinned objects
    eTransform offset;
    offset.setScale(glm::vec3(100.f));
    offset.setRotation(glm::quat(0.707f, -0.707f, 0.f, 0.f));
    m_renderSource = RenderSourceBuilder()
                         .setTransform(transform)
                         .setOffset(offset)
                         .setModel(m_model)
                         .setAnimator(m_animator)
                         .build();
    m_renderManager->addSource(m_renderSource);
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
    else
        // TODO: only when required
        m_ragdoll->syncFromAnimation(m_renderSource->transform.getModelMatrix());

    // update animation
    m_animator->update(deltaTime);

    // update pistol-aim blend
    m_aimBlend += deltaTime * m_aimStateChangeSpeed * (m_controller->m_aimLocked ? 1.f : -1.f);
    m_aimBlend = std::max(0.0f, std::min(m_aimBlend, 1.0f));
    updateAimPoseBlendMask(m_aimBlend);

    // update firing blend
    Anim &firingPose = *m_animPoseFiring;
    firingPose.m_blendFactor += deltaTime * m_firingStateChangeSpeed * (m_firing ? 1.f : -1.f);
    firingPose.m_blendFactor = std::max(0.0f, std::min(firingPose.m_blendFactor, 1.0f));

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

        updateMoveOrient();
        updateMoveStage();

        m_idle->m_blendFactor = m_moveStage.idle;
        updateMoveCircleBlend(m_walkCircle, m_moveStage.walk);
        updateMoveCircleBlend(m_runCircle, m_moveStage.run);

        // full body rotating
        if (!m_controller->m_aimLocked && m_controller->m_dotFront < 0.f)
        {
            // reset animation
            if (m_animTurn180->m_blendFactor < 0.01f)
            {
                m_animTurn180->m_timer = 0.f;
            }

            interpolateValue(m_animTurn180->m_blendFactor, 1.f);
        }
        else
            interpolateValue(m_animTurn180->m_blendFactor, 0.f);

        interpolateValue(m_animPoseLeanLeft->m_blendFactor, std::clamp(-m_controller->m_turnFactor, 0.f, 1.f));
        interpolateValue(m_animPoseLeanRight->m_blendFactor, std::clamp(m_controller->m_turnFactor, 0.f, 1.f));

        m_prevMoveStage.idle = m_moveStage.idle;
        m_prevMoveStage.walk = m_moveStage.walk;
        m_prevMoveStage.run = m_moveStage.run;
    }

    updateModelMatrix();

    updateHeadFollow(deltaTime);

    // NOTE: should after updateModelMatrix
    // update ragdoll blend
    Anim &ragdolPose = *m_animPoseRagdoll;
    ragdolPose.m_blendFactor = (m_ragdollActive ? 1.f : -1.f);
    // ragdolPose.blendFactor += deltaTime * m_stateChangeSpeed * (m_ragdollActive ? 1.f : -1.f);
    ragdolPose.m_blendFactor = std::max(0.0f, std::min(ragdolPose.m_blendFactor, 1.0f));
}

void Character::updateMoveOrient()
{
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

        // TODO: sync timer
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

        // TODO: sync timer
    }

    interpolateValue(m_moveOrient.forward, front);
    interpolateValue(m_moveOrient.back, back);
    interpolateValue(m_moveOrient.left, left);
    interpolateValue(m_moveOrient.right, right);
    interpolateValue(m_moveOrient.backLeft, backLeft);
    interpolateValue(m_moveOrient.backRight, backRight);
}

// TODO: stepper
void Character::updateMoveStage()
{
    float fullIdleEnd = m_moveStage.idleEndGap;
    float fullWalkStart = m_controller->m_maxWalkRelative - m_moveStage.walkStartGap;
    float fullWalkEnd = m_controller->m_maxWalkRelative + m_moveStage.walkEndGap;
    float fullRunStart = m_controller->m_maxRunRelative - m_moveStage.runStartGap;

    if (m_controller->m_verticalSpeed <= fullIdleEnd)
    {
        m_moveStage.idle = 1.f;
        m_moveStage.walk = 0.f;
        m_moveStage.run = 0.f;
    }
    else if (m_controller->m_verticalSpeed <= fullWalkStart)
    {
        // blend idle -> walk
        float blendRange = fullWalkStart - fullIdleEnd;
        float blendFactor = m_controller->m_verticalSpeed - fullIdleEnd;
        m_moveStage.walk = blendFactor / blendRange;
        m_moveStage.idle = 1.f - m_moveStage.walk;

        m_moveStage.run = 0.f;

        // TODO: update footstep frequency
    }
    else if (m_controller->m_verticalSpeed <= fullWalkEnd)
    {
        m_moveStage.idle = 0.f;
        m_moveStage.walk = 1.f;
        m_moveStage.run = 0.f;
    }
    else if (m_controller->m_verticalSpeed <= fullRunStart)
    {
        m_moveStage.idle = 0.f;

        // blend walk -> run
        float blendRange = fullRunStart - fullWalkEnd;
        float blendFactor = m_controller->m_verticalSpeed - fullWalkEnd;
        m_moveStage.run = blendFactor / blendRange;
        m_moveStage.walk = 1.f - m_moveStage.run;

        // TODO: update footstep frequency
    }
    else if (m_controller->m_verticalSpeed > fullRunStart)
    {
        m_moveStage.idle = 0.f;
        m_moveStage.walk = 0.f;
        m_moveStage.run = 1.f;
    }

    syncFootstepFrequency();
}

void Character::interpolateValue(float &value, float newValue)
{
    value = CommonUtil::lerp(value, newValue, m_blendSpeed);
}

void Character::updateMoveCircleBlend(MoveCircle &circle, float value)
{
    interpolateValue(circle.m_forward->m_blendFactor, m_moveOrient.forward * value);
    interpolateValue(circle.m_back->m_blendFactor, m_moveOrient.back * value);
    interpolateValue(circle.m_left->m_blendFactor, m_moveOrient.left * value);
    interpolateValue(circle.m_right->m_blendFactor, m_moveOrient.right * value);
    interpolateValue(circle.m_backLeft->m_blendFactor, m_moveOrient.backLeft * value);
    interpolateValue(circle.m_backRight->m_blendFactor, m_moveOrient.backRight * value);
}

void Character::updateMoveCircleTimer(MoveCircle &circle, float value)
{
    circle.m_forward->m_timer = m_moveOrient.forward * value;
    circle.m_back->m_timer = m_moveOrient.back * value;
    circle.m_left->m_timer = m_moveOrient.left * value;
    circle.m_right->m_timer = m_moveOrient.right * value;
    circle.m_backLeft->m_timer = m_moveOrient.backLeft * value;
    circle.m_backRight->m_timer = m_moveOrient.backRight * value;
}

void Character::updateMoveCirclePlaybackSpeed(MoveCircle &circle, float value)
{
    circle.m_forward->m_playbackSpeed = value;
    circle.m_back->m_playbackSpeed = value;
    circle.m_left->m_playbackSpeed = value;
    circle.m_right->m_playbackSpeed = value;
    circle.m_backLeft->m_playbackSpeed = value;
    circle.m_backRight->m_playbackSpeed = value;
}

void Character::updateHeadFollow(float deltaTime)
{
    // update AnimPose blend
    Anim &animPose = *m_animPoseHeadFollow;
    animPose.m_blendFactor += deltaTime * m_aimStateChangeSpeed * (m_headFollow ? 1.f : -1.f);
    animPose.m_blendFactor = std::max(0.0f, std::min(animPose.m_blendFactor, 1.0f));

    Bone *bone = animPose.m_animation->getBone("mixamorig:Head");
    glm::quat &boneRot = bone->m_rotations[0].value;

    bool shouldSlerp = true;
    if (m_headFollow)
        shouldSlerp = updateHeadFollowRotation(!m_lastHeadFollow);
    else
        m_clampedHeadRot = glm::quat(1.f, 0.f, 0.f, 0.f);

    shouldSlerp = shouldSlerp && animPose.m_blendFactor != 0.f;

    boneRot = shouldSlerp ? glm::slerp(boneRot, m_clampedHeadRot, 0.1f) : m_clampedHeadRot;
    bone->updatePose();

    m_lastHeadFollow = m_headFollow;
}

bool Character::updateHeadFollowRotation(bool firstUpdate)
{
    m_neckRot = getNeckRotation();
    glm::quat characterRot = glm::quatLookAtRH(-m_controller->m_lookDir, glm::vec3(0.f, 1.f, 0.f));
    glm::quat cameraRot = glm::quatLookAtRH(-m_followCamera->front, m_followCamera->up);

    // worldCamera = worldCharacter * localNeck * localHead
    m_headRot = glm::normalize(glm::inverse(characterRot * m_neckRot) * cameraRot * m_headRotOffset);

    // clamp head rotation
    float angle = glm::angle(m_headRot);
    if (angle > M_PI)
        angle = fmod(M_PI * 2.f - angle, 3.14159265f);

    const float maxHeadAngle = M_PI_2 - 0.3f;
    const float minSafeZoneAngle = M_PI - 0.3f;

    if (!firstUpdate && angle > minSafeZoneAngle)
    {
        // prevent constant flip by keeping the last clamped value
    }
    else if (angle > maxHeadAngle)
    {
        float factor = maxHeadAngle / angle;
        const glm::quat identity(1.f, 0.f, 0.f, 0.f);
        m_clampedHeadRot = glm::slerp(identity, m_headRot, factor);
    }
    else
    {
        m_clampedHeadRot = m_headRot;
    }

    bool shouldSlerp = angle > maxHeadAngle;
    return shouldSlerp;
}

glm::quat Character::getNeckRotation()
{
    int neckId = m_animator->m_animations[0]->m_boneInfoMap["mixamorig:Neck"].id;
    glm::mat4 neckModel = m_animator->m_globalMatrices[neckId];

    glm::vec3 position, scale;
    glm::quat rotation;
    CommonUtil::decomposeModel(neckModel, position, rotation, scale);
    // glm::decompose(neckModel, scale, rotation, position, skew, perspective);

    return rotation;
}

void Character::updateModelMatrix()
{
    glm::mat4 model = glm::mat4(1.0f);

    {
        glm::vec3 position = m_position;
        // if (m_controller->m_moving)
        //     position.y -= m_controller->m_floatElevation;

        model = glm::translate(model, position);
        model = glm::rotate(model, m_rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, m_rotation.y * (1.0f - m_animPoseRagdoll->m_blendFactor), glm::vec3(0, 1, 0));
        model = glm::rotate(model, m_rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(m_scale));
    }

    m_modelMatrix = model;

    m_renderSource->transform.setModelMatrix(model);
    m_renderSource->updateModelMatrix();
}

void Character::syncFootstepFrequency2()
{
    m_walkPerc = m_walkCircle.m_forward->m_timer / m_walkCircle.m_forward->m_animation->m_duration;
    m_runPerc = m_runCircle.m_forward->m_timer / m_runCircle.m_forward->m_animation->m_duration;

    m_walkAnimSpeed = (m_walkStepFreq + (m_runStepFreq - m_walkStepFreq) * m_moveStage.run) / m_walkStepFreq;
    m_runAnimSpeed = (m_walkStepFreq + (m_runStepFreq - m_walkStepFreq) * m_moveStage.run) / m_runStepFreq;

    updateMoveCirclePlaybackSpeed(m_walkCircle, m_walkAnimSpeed);
    updateMoveCirclePlaybackSpeed(m_runCircle, m_runAnimSpeed);
}

void Character::syncFootstepFrequency()
{
    if (m_moveStage.walk == 1.f)
    {
        // float walkSpeedRange = m_controller->m_verticalSpeed / (m_controller->m_maxWalkRelative + m_moveStage.walkEndGap);
        // updateMoveCirclePlaybackSpeed(m_walkCircle, walkSpeedRange);

        updateMoveCirclePlaybackSpeed(m_walkCircle, 1.f);
    }
    else if (m_moveStage.run == 1.f)
    {
        updateMoveCirclePlaybackSpeed(m_runCircle, 1.f);
    }
    else if (m_moveStage.run > 0.f && m_moveStage.run < 1.f)
    {
        m_walkPerc = m_walkCircle.m_forward->m_timer / m_walkCircle.m_forward->m_animation->m_duration;
        m_runPerc = m_runCircle.m_forward->m_timer / m_runCircle.m_forward->m_animation->m_duration;

        m_walkAnimSpeed = (m_walkStepFreq + (m_runStepFreq - m_walkStepFreq) * m_moveStage.run) / m_walkStepFreq;
        m_runAnimSpeed = (m_walkStepFreq + (m_runStepFreq - m_walkStepFreq) * m_moveStage.run) / m_runStepFreq;

        updateMoveCirclePlaybackSpeed(m_walkCircle, m_walkAnimSpeed);
        updateMoveCirclePlaybackSpeed(m_runCircle, m_runAnimSpeed);

        // refresh to fix error gap
        float timerGap = std::fabs(m_walkPerc - m_runPerc);
        bool timerOff = timerGap > 0.01f;

        // if (timerOff)
        //     std::cout << "timerOff: timerGap: " << timerGap << std::endl;

        if (m_prevMoveStage.run == 0.f || (timerOff && m_moveStage.run < 0.5f))
        {
            float runTimer = m_walkPerc * m_runCircle.m_forward->m_animation->m_duration;
            updateMoveCircleTimer(m_runCircle, runTimer);
        }
        else if (m_prevMoveStage.run == 1.f || timerOff)
        {
            float walkTimer = m_runPerc * m_walkCircle.m_forward->m_animation->m_duration;
            updateMoveCircleTimer(m_walkCircle, walkTimer);
        }
    }

    // return to idle
    /* float gap = m_moveStage.idle - m_prevMoveStage.idle;
    if (!m_controller->m_moving && m_moveStage.idle != 1.f && gap > 0.01f) // stopping
    {
        float nextTime;
        float fullTime = m_walkCircle.m_forward->m_animation->m_duration;
        float halfTime = fullTime / 2.f;

        float time = m_walkCircle.m_forward->m_timer;

        // TODO: better sync
        if (time > halfTime)
            nextTime = fullTime;
        else
            nextTime = halfTime;

        updateMoveCircleTimer(m_walkCircle, nextTime);

        // TODO: stop character at stop anim position
    } */
}

void Character::activateRagdoll()
{
    inactivateCollider();
    btVector3 modelPos = BulletGLM::getBulletVec3(m_position);
    // TODO: keep velocity?
    m_ragdoll->unFreezeBodies();
    m_ragdoll->changeState(RagdollState::loose);
    m_ragdollActive = true;
}

void Character::applyImpulseFullRagdoll(glm::vec3 impulse)
{
    btVector3 imp = BulletGLM::getBulletVec3(impulse * (1.f / BODYPART_COUNT));
    for (int i = 0; i < BODYPART_COUNT; i++)
    {
        btRigidBody *rb = m_ragdoll->m_bodies[i];
        rb->applyCentralImpulse(imp);
    }
}

void Character::applyImpulseChest(glm::vec3 impulse)
{
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
        activateRagdoll();
        applyImpulseFullRagdoll(-BulletGLM::getGLMVec3(totalForce) * m_ragdolActivateFactor);
    }
}

// TODO: single run in a thread
void Character::updateAimPoseBlendMask(float blendFactor)
{
    // early exit
    Bone *bone = m_animPosePistolAim->m_animation->getBone("mixamorig:Head");
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

    m_animPosePistolAim->m_animation->setBlendMask(blendMask, 0.0f);
}
