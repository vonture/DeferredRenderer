#pragma once

#include "Defines.h"
#include "ConfigurationPaneBase.h"

template <class T>
class ConfigurationPane : public ConfigurationPaneBase
{
private:
	T* _obj;

protected:
	T* GetConfiguredObject() { return _obj; }

public:
	ConfigurationPane(Gwen::Controls::Base* parent, const WCHAR* name, T* obj)
		: ConfigurationPaneBase(parent, name), _obj(obj)
	{
	}
};