#ifndef system_monitor_ui_hpp
#define system_monitor_ui_hpp

#include <mach/mach.h>

#include "../base_ui.h"

class SystemMonitorUI : public BaseUI
{
private:
    task_basic_info *m_info;

public:
    SystemMonitorUI(task_basic_info *info) : m_info(info) {}

    void render() override;
};

#endif /* system_monitor_ui_hpp */
