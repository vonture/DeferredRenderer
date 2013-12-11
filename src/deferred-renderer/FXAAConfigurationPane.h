#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "FXAAPostProcess.h"
#include "SliderWithLabel.h"

class FXAAConfigurationPane : public ConfigurationPane<FXAAPostProcess>
{
private:
    SliderWithLabel* _qualityPresetSlider;
    SliderWithLabel* _subpixelSlider;
    SliderWithLabel* _edgeThresholdSlider;
    SliderWithLabel* _edgeThresholdMinSlider;

    virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
    FXAAConfigurationPane(Gwen::Controls::Base* parent, FXAAPostProcess* pp);

    void OnFrameMove(double totalTime, float dt);
};