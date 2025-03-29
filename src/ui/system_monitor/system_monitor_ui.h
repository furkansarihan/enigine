#ifndef system_monitor_ui_hpp
#define system_monitor_ui_hpp

#include "../../timer/timer.h"
#include "../../update_manager/update_manager.h"
#include "../../utils/common.h"
#include "../base_ui.h"

class SystemMonitorUI : public BaseUI, public Updatable
{
public:
    SystemMonitorUI()
    {
    }

    uint64_t m_ramUsage;
    Timer m_timer;

    void render() override;
    void update(float deltaTime) override;
};

#endif /* system_monitor_ui_hpp */
