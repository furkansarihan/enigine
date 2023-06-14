#ifndef vehicle_ui_hpp
#define vehicle_ui_hpp

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../base_ui.h"
#include "../../vehicle/vehicle.h"
#include "../../utils/bullet_glm.h"

struct Follow
{
    glm::vec3 offset = glm::vec3(0.0f, 2.5f, 0.0f);
    float distance = 15.0f;
    float gap = 0.0f;
    float gapTarget = 0.0f;
    float gapFactor = 0.1f;
    float gapSpeed = 0.002f;
    float steeringFactor = 30.0f;
    float angleFactor = 0.1f;
    float angleSpeed = 0.001f;
};

class VehicleUI : public BaseUI
{
private:
    Vehicle *m_vehicle;

public:
    VehicleUI(Vehicle *vehicle);

    float m_scale = 0.028f;
    float m_wheelScale = 0.028f;
    glm::vec3 m_bodyOffset = glm::vec3(0.f, 2.07f, -0.14f);
    glm::vec3 m_wheelOffset = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 m_rotation = glm::vec3(0.f, -M_PI_2, 0.f);
    glm::vec3 m_wheelRotation = glm::vec3(0.f, -M_PI_2, 0.f);
    glm::vec3 m_bodyRotation = glm::vec3(0.f, 0.f, -0.020f);

    glm::vec3 m_hoodOffset = glm::vec3(0.f, 1.87f, 3.12f);
    glm::vec3 m_trunkOffset = glm::vec3(0.f, 2.15f, -3.89f);
    glm::vec3 m_doorOffsets[4];

    bool m_controlVehicle = true;
    bool m_followVehicle = true;

    glm::vec3 m_pos = glm::vec3(0.0f, 2.5f, 0.0f);
    Follow m_follow;

    void render() override;
    void renderVec3(const char *header, glm::vec3 &vec, float dragSpeed);
    void updateWheelInfo();
};

#endif /* vehicle_ui_hpp */
