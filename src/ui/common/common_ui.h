#ifndef common_ui_hpp
#define common_ui_hpp

#include "../../external/imgui/imgui.h"
#include "../../timer/timer.h"

class CommonUI
{
public:
    static void DrawTimerWidget(const Timer &timer, const char *widgetTitle = "Timer Stats");
};

#endif /* common_ui_hpp */
