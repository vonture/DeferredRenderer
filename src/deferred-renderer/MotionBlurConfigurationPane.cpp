#include "PCH.h"
#include "MotionBlurConfigurationPane.h"

MotionBlurConfigurationPane::MotionBlurConfigurationPane(Gwen::Controls::Base* parent, MotionBlurPostProcess* pp)
    : ConfigurationPane(parent, L"Motion Blur", pp)
{
}

void MotionBlurConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
}

void MotionBlurConfigurationPane::OnFrameMove(double totalTime, float dt)
{
}