#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "HBAOPostProcess.h"
#include "SliderWithLabel.h"

class HBAOConfigurationPane : public ConfigurationPane<HBAOPostProcess>
{
private:
    virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
    HBAOConfigurationPane(Gwen::Controls::Base* parent, HBAOPostProcess* pp);

    void OnFrameMove(double totalTime, float dt);
};