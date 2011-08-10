#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "MotionBlurPostProcess.h"
#include "SliderWithLabel.h"

class MotionBlurConfigurationPane : public ConfigurationPane<MotionBlurPostProcess>
{
private:
	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	MotionBlurConfigurationPane(Gwen::Controls::Base* parent, MotionBlurPostProcess* pp);

	void OnFrameMove(double totalTime, float dt);
};