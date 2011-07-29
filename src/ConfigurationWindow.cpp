#include "ConfigurationWindow.h"

ConfigurationWindow::ConfigurationWindow(Gwen::Controls::Base* parent)
	: Gwen::Controls::WindowControl(parent), _configSelectComboBox(NULL), _selectedPane(NULL)
{
	SetTitle(L"Configuration");
	SetClosable(false);
	SetMargin(Gwen::Margin(2, 2, 2, 2));
	DisableResizing();

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
	newPane->RestrictToParent(true);
	newPane->Hide();
}

void ConfigurationWindow::RemoveConfigPane(ConfigurationPaneBase* removePane)
{
}

void ConfigurationWindow::OnFrameMove(double totalTime, float dt)
{
	Gwen::Controls::Label* selectedLabel = _configSelectComboBox->GetSelectedItem();

	if (selectedLabel)
	{
		Gwen::UnicodeString selectedLabelString = selectedLabel->GetText();
		if (_paneMap.find(selectedLabelString) != _paneMap.end())
		{
			if (_selectedPane != _paneMap[selectedLabelString])
			{
				if (_selectedPane)
				{
					_selectedPane->Hide();
				}

				_selectedPane = _paneMap[selectedLabelString];
				_selectedPane->Show();
			}
		}
	}	

	std::map<Gwen::UnicodeString, ConfigurationPaneBase*>::iterator it;
	for (it = _paneMap.begin(); it != _paneMap.end(); it++)
	{
		it->second->OnFrameMove(totalTime, dt);
	}
}