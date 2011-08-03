#pragma once 

#include "Defines.h"
#include "ConfigurationPane.h"
#include "Logger.h"

class ProfilePane : public ConfigurationPane<Logger>
{
public:
	ProfilePane(Gwen::Controls::Base* parent, Logger* logger);
	
	void OnFrameMove(double totalTime, float dt);
};