#pragma once 

#include "Defines.h"
#include "IUpdateable.h"
#include "ConfigurationPaneBase.h"
#include "Gwen\Controls\WindowControl.h"
#include "Gwen\Controls\ComboBox.h"
#include "Gwen\Controls\GroupBox.h"

class ConfigurationWindow : public IUpdateable, public Gwen::Controls::WindowControl
{
private:
	Gwen::Controls::ComboBox* _configSelectComboBox;

	ConfigurationPaneBase* _selectedPane;
	std::map<Gwen::Controls::MenuItem*, ConfigurationPaneBase*> _paneMap;

public:
	ConfigurationWindow(Gwen::Controls::Base* parent);
	~ConfigurationWindow();

	void AddConfigPane(ConfigurationPaneBase* newPane);
	void RemoveConfigPane(ConfigurationPaneBase* removePane);

	void OnSelectionChanged(Gwen::Controls::Base* pFromPanel);

	void OnFrameMove(double totalTime, float dt);
};