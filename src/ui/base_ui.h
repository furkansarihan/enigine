#ifndef base_ui_hpp
#define base_ui_hpp

#include "../external/imgui/imgui.h"

class BaseUI
{
public:
    virtual ~BaseUI() {}
    virtual void render() = 0;

    bool m_collapsed = true;
};

#endif /* base_ui_hpp */
