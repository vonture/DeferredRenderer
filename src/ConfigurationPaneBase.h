#pragma once

#include "Defines.h"
#include "IUpdateable.h"
#include "Gwen/Controls/GroupBox.h"

class ConfigurationPaneBase : public IUpdateable, public Gwen::Controls::GroupBox
{
public:
	ConfigurationPaneBase(Gwen::Controls::Base* parent) : Gwen::Controls::GroupBox(parent) { }
	
	virtual void OnFrameMove(double totalTime, float dt) = 0;
};