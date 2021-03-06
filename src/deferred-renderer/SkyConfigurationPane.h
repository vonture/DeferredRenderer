#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "SkyPostProcess.h"
#include "Gwen/Controls/ColorPicker.h"
#include "Gwen/Controls/CheckBox.h"
#include "SliderWithLabel.h"
#include "DirectionSelector.h"

class SkyConfigurationPane : public ConfigurationPane<SkyPostProcess>
{
private:
    SliderWithLabel* _skyTypeSlider;

    Gwen::Controls::Label* _skyColorLabel;
    Gwen::Controls::ColorPicker* _skyColorPicker;
    SliderWithLabel* _skyBrightnessSlider;

    Gwen::Controls::CheckBoxWithLabel* _sunEnabledCheckBox;

    Gwen::Controls::Label* _sunColorLabel;
    Gwen::Controls::ColorPicker* _sunColorPicker;

    SliderWithLabel* _sunBrightnessSlider;

    Gwen::Controls::Label* _sunWidthLabel;
    SliderWithLabel* _sunWidthSlider;

    Gwen::Controls::Label* _sunDirLabel;
    DirectionSelector* _sunDirSelector;

    virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
    SkyConfigurationPane(Gwen::Controls::Base* parent, SkyPostProcess* pp);
    ~SkyConfigurationPane();

    void OnFrameMove(double totalTime, float dt);
};