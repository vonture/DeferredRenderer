#pragma once

#include "PCH.h"
#include "IUpdateable.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Utility.h"

class ConfigurationPaneBase : public Gwen::Controls::Base, public IUpdateable
{
public:
	ConfigurationPaneBase(Gwen::Controls::Base* parent, const WCHAR* name) 
		: Gwen::Controls::Base(parent) 
	{ 
		SetName(Gwen::Utility::UnicodeToString(name));
	}

	virtual void OnFrameMove(double totalTime, float dt) { };
};