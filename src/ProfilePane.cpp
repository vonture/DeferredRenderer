#include "ProfilePane.h"

ProfilePane::ProfilePane(Gwen::Controls::Base* parent, Logger* logger)
	: ConfigurationPane(parent, logger)
{
	SetName("Profile");
}

void ProfilePane::OnFrameMove(double totalTime, float dt)
{
}