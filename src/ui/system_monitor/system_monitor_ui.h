#ifndef system_monitor_ui_hpp
#define system_monitor_ui_hpp

#include "../base_ui.h"
#include "../../update_manager/update_manager.h"
#include "../../utils/common.h"

class SystemMonitorUI : public BaseUI, public Updatable
{
private:

public:
    SystemMonitorUI() {}

    uint64_t m_ramUsage;

    void render() override;
    void update(float deltaTime) override;
};

#endif /* system_monitor_ui_hpp */
