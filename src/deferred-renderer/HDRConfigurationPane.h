#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "HDRPostProcess.h"
#include "SliderWithLabel.h"

class HDRConfigurationPane : public ConfigurationPane<HDRPostProcess>
{
private:
    SliderWithLabel* _tauSlider;
    SliderWithLabel* _whiteLumPercSlider;
    SliderWithLabel* _bloomThresholdSlider;
    SliderWithLabel* _bloomMagnitudeSlider;
    SliderWithLabel* _bloomBlurSigmaSlider;
    SliderWithLabel* _exposureKeySlider;
    SliderWithLabel* _timeScaleSlider;

    virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
    HDRConfigurationPane(Gwen::Controls::Base* parent, HDRPostProcess* pp);

    void OnFrameMove(double totalTime, float dt);
};