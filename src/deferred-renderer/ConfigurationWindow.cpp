#include "PCH.h"
#include "ConfigurationWindow.h"

ConfigurationWindow::ConfigurationWindow(Gwen::Controls::Base* parent)
    : Gwen::Controls::WindowControl(parent), _configSelectComboBox(NULL), _selectedPane(NULL)
{
    SetTitle(L"Configuration");
    SetClosable(false);

    SetPadding(Gwen::Margin(1, 1, 1, 1));

    _configSelectComboBox = new Gwen::Controls::ComboBox(this);
    _configSelectComboBox->Dock(Gwen::Pos::Top);
}

void ConfigurationWindow::onComboBoxSelect(Gwen::Controls::Base* control)
{
    Gwen::Controls::MenuItem* newSelectedItem = dynamic_cast<Gwen::Controls::MenuItem*>(control);
    if (newSelectedItem && _paneMap.find(newSelectedItem) != _paneMap.end())
    {
        if (_selectedPane != _paneMap[newSelectedItem])
        {
            if (_selectedPane)
            {
                _selectedPane->Hide();
            }

            _selectedPane = _paneMap[newSelectedItem];
            _selectedPane->Show();
        }
    }
}

void ConfigurationWindow::AddChild(Gwen::Controls::Base* pChild)
{
    _newChildren.push_back(pChild);

    Gwen::Controls::WindowControl::AddChild(pChild);
}

void ConfigurationWindow::addConfigPane(ConfigurationPaneBase* newPane)
{
    newPane->SetParent(this);

    Gwen::UnicodeString label = Gwen::Utility::StringToUnicode(newPane->GetName());

    Gwen::Controls::MenuItem* item = _configSelectComboBox->AddItem(label);
    item->onMenuItemSelected.Add(this, &ConfigurationWindow::onComboBoxSelect);

    _paneMap[item] = newPane;

    newPane->Dock(Gwen::Pos::Fill);
    newPane->Hide();

    if (_paneMap.size() == 1)
    {
        // Select the first pane
        onComboBoxSelect(item);
    }
}

void ConfigurationWindow::OnFrameMove(double totalTime, float dt)
{
    if (_newChildren.size() > 0)
    {
        for (UINT i = 0; i < _newChildren.size(); i++)
        {
            ConfigurationPaneBase* asPane = dynamic_cast<ConfigurationPaneBase*>(_newChildren[i]);
            if (asPane)
            {
                addConfigPane(asPane);
            }
        }

        _newChildren.clear();
    }

    std::map<Gwen::Controls::MenuItem*, ConfigurationPaneBase*>::iterator it;
    for (it = _paneMap.begin(); it != _paneMap.end(); it++)
    {
        it->second->OnFrameMove(totalTime, dt);
    }
}