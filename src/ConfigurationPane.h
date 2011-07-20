#pragma once

#include "Defines.h"
#include "ConfigurationPaneBase.h"

template <class T>
class ConfigurationPane : public ConfigurationPaneBase
{
private:
	T* _obj;

public:
	ConfigurationPane(Gwen::Controls::Base* parent, T* obj)
		: ConfigurationPaneBase(parent), _obj(obj)
	{
	}

	T* GetConfiguredObject() { return _obj; }
};