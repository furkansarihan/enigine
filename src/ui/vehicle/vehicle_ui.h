#ifndef vehicle_ui_hpp
#define vehicle_ui_hpp

#include "btBulletDynamicsCommon.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../base_ui.h"
#include "../../vehicle/vehicle.h"
#include "../../utils/bullet_glm.h"
#include "../../car_controller/car_controller.h"

class VehicleUI : public BaseUI
{
private:
    CarController *m_cController;
    Vehicle *m_vehicle;

public:
    VehicleUI(CarController *m_cController, Vehicle *vehicle);

    void render() override;
    void renderCompoundShapeEditor(const char *header, btCompoundShape *compoundShape);
    void renderHingeState(int index);
    void updateWheelInfo();
};

#endif /* vehicle_ui_hpp */
