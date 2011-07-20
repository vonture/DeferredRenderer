#include "ConfigurationWindow.h"

ConfigurationWindow::ConfigurationWindow(Gwen::Controls::Base* parent)
	: Gwen::Controls::WindowControl(parent), _configSelectComboBox(NULL), _selectedPane(NULL)
{
	SetTitle(L"Configuration");
	SetClosable(false);
	SetMargin(Gwen::Margin(2, 2, 2, 2));

	_configSelectComboBox = new Gwen::Controls::ComboBox(this);
	_configSelectComboBox->SetPadding(Gwen::Padding(2, 2, 2, 2));
	_configSelectComboBox->SetMargin(Gwen::Margin(2, 2, 2, 2));
	_configSelectComboBox->Dock(Gwen::Pos::Top);
}

ConfigurationWindow::~ConfigurationWindow()
{
}

void ConfigurationWindow::AddConfigPane(ConfigurationPaneBase* newPane)
{
	newPane->SetParent(this);
	
	Gwen::UnicodeString label = Gwen::Utility::StringToUnicode(newPane->GetName());

	Gwen::Controls::MenuItem* menuItem = _configSelectComboBox->AddItem(label, newPane->GetName() + " label");
	menuItem->onMenuItemSelected.Add(this, &ConfigurationWindow::OnSelectionChanged);
	
	_paneMap[menuItem] = newPane;

	newPane->SetPadding(Gwen::Padding(2, 2, 2, 2));
	newPane->SetMargin(Gwen::Margin(2, 2, 2, 2));
	newPane->Dock(Gwen::Pos::Fill);
	newPane->Hide();

	if (!_selectedPane)
	{
		OnSelectionChanged(menuItem);
	}
}

void ConfigurationWindow::RemoveConfigPane(ConfigurationPaneBase* removePane)
{
}

void ConfigurationWindow::OnSelectionChanged(Gwen::Controls::Base* pFromPanel)
{
	Gwen::Controls::MenuItem* selectedMenuItem = (Gwen::Controls::MenuItem*)pFromPanel;

	if (_selectedPane)
	{
		_selectedPane->Hide();
	}

	if (_paneMap.find(selectedMenuItem) != _paneMap.end())
	{
		_paneMap[selectedMenuItem]->Show();
		_selectedPane = _paneMap[selectedMenuItem];
	}
	else
	{
		_selectedPane = NULL;
	}
}

void ConfigurationWindow::OnFrameMove(double totalTime, float dt)
{
	std::map<Gwen::Controls::MenuItem*, ConfigurationPaneBase*>::iterator it;
	for (it = _paneMap.begin(); it != _paneMap.end(); it++)
	{
		it->second->OnFrameMove(totalTime, dt);
	}
}