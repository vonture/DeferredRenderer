#include "PCH.h"
#include "HBAOConfigurationPane.h"

HBAOConfigurationPane::HBAOConfigurationPane(Gwen::Controls::Base* parent, HBAOPostProcess* pp)
    : ConfigurationPane(parent, L"HBAO", pp)
{
}

void HBAOConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
}

void HBAOConfigurationPane::OnFrameMove(double totalTime, float dt)
{
}