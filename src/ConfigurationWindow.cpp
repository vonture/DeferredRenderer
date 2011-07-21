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
	Gwen::UnicodeString label = Gwen::Utility::StringToUnicode(newPane->GetName());
	
	_configSelectComboBox->AddItem(label, newPane->GetName() + " label");	
	_paneMap[label] = newPane;

	newPane->SetPadding(Gwen::Padding(2, 2, 2, 2));
	newPane->SetMargin(Gwen::Margin(2, 2, 2, 2));
	newPane->Dock(Gwen::Pos::Fill);
	newPane->Hide();
}

void ConfigurationWindow::RemoveConfigPane(ConfigurationPaneBase* removePane)
{
}

void ConfigurationWindow::OnFrameMove(double totalTime, float dt)
{
	Gwen::UnicodeString selectedLabel = _configSelectComboBox->GetSelectedItem()->GetText();	
	if (_paneMap.find(selectedLabel) != _paneMap.end())
	{
		if (_selectedPane != _paneMap[selectedLabel])
		{
			if (_selectedPane)
			{
				_selectedPane->Hide();
			}

			_selectedPane = _paneMap[selectedLabel];
			_selectedPane->Show();
		}
	}

	std::map<Gwen::UnicodeString, ConfigurationPaneBase*>::iterator it;
	for (it = _paneMap.begin(); it != _paneMap.end(); it++)
	{
		it->second->OnFrameMove(totalTime, dt);
	}
}