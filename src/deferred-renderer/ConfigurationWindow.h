#pragma once

#include "PCH.h"
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
    std::map<Gwen::Controls::MenuItem*, ConfigurationPaneBase*> _paneMap;

    std::vector<Gwen::Controls::Base*> _newChildren;

    void addConfigPane(ConfigurationPaneBase* newPane);

    virtual void onComboBoxSelect(Gwen::Controls::Base* control);

protected:
    virtual void AddChild(Gwen::Controls::Base* pChild);

public:
    ConfigurationWindow(Gwen::Controls::Base* parent);

    void OnFrameMove(double totalTime, float dt);
};