#pragma once 

#include "Defines.h"
#include "IUpdateable.h"
#include "ConfigurationPaneBase.h"
#include "Gwen\Controls\WindowControl.h"
#include "Gwen\Controls\ComboBox.h"
#include "Gwen\Controls\GroupBox.h"

class ConfigurationWindow :  public Gwen::Controls::WindowControl, public IUpdateable
{
private:
	Gwen::Controls::ComboBox* _configSelectComboBox;

	ConfigurationPaneBase* _selectedPane;
	std::map<Gwen::UnicodeString, ConfigurationPaneBase*> _paneMap;

public:
	ConfigurationWindow(Gwen::Controls::Base* parent);
	~ConfigurationWindow();

	void AddConfigPane(ConfigurationPaneBase* newPane);
	void RemoveConfigPane(ConfigurationPaneBase* removePane);
	
	void OnFrameMove(double totalTime, float dt);
};