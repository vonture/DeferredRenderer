#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "TestingCamera.h"
#include "SliderWithLabel.h"
#include "Gwen/Controls/HorizontalSlider.h"

class CameraConfigurationPane : public ConfigurationPane<TestingCamera>
{
private:
    Gwen::Controls::Label* _cameraPositionLabel;
    Gwen::Controls::Label* _cameraRotationLabel;

    SliderWithLabel* _cameraFovSlider;
    SliderWithLabel* _cameraNearClipSlider;
    SliderWithLabel* _cameraFarClipSlider;
    SliderWithLabel* _cameraMoveSpeedSlider;
    SliderWithLabel* _cameraRotSpeedSlider;

    virtual void onValueChanged(Gwen::Controls::Base* controls);

public:
    CameraConfigurationPane(Gwen::Controls::Base* parent, TestingCamera* cam);

    void OnFrameMove(double totalTime, float dt);
};