#ifndef vehicle_ui_hpp
#define vehicle_ui_hpp

#include "../base_ui.h"
#include "../../vehicle/vehicle.h"

class VehicleUI : public BaseUI
{
private:
    Vehicle *m_vehicle;

public:
    VehicleUI(Vehicle *vehicle) : m_vehicle(vehicle) {}

    void render() override;
};

#endif /* vehicle_ui_hpp */
