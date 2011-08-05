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
	Gwen::Controls::Layout::Table* _layout;

	Gwen::Controls::Layout::TableRow* _paneRow;

	ConfigurationPaneBase* _selectedPane;
	std::map<Gwen::Controls::MenuItem*, ConfigurationPaneBase*> _paneMap;

	virtual void onComboBoxSelect(Gwen::Controls::Base* control);

protected:
	virtual void AddChild(Gwen::Controls::Base* pChild);

public:
	ConfigurationWindow(Gwen::Controls::Base* parent);

	void AddConfigPane(ConfigurationPaneBase* newPane);
	void RemoveConfigPane(ConfigurationPaneBase* removePane);
	
	void OnFrameMove(double totalTime, float dt);
};