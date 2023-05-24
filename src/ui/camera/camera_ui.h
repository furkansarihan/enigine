#ifndef camera_ui_hpp
#define camera_ui_hpp

#include "../base_ui.h"
#include "../../camera/camera.h"

class CameraUI : public BaseUI
{
private:
    Camera *m_camera;

public:
    CameraUI(Camera *camera) : m_camera(camera) {}

    void render() override;
};

#endif /* camera_ui_hpp */
