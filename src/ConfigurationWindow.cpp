#include "ConfigurationWindow.h"

ConfigurationWindow::ConfigurationWindow(Gwen::Controls::Base* parent)
	: Gwen::Controls::WindowControl(parent), _configSelectComboBox(NULL), _selectedPane(NULL)
{
	SetTitle(L"Configuration");
	SetClosable(false);

	_layout = new Gwen::Controls::Layout::Table(this);
	_layout->SetColumnCount(1);
	_layout->Dock(Gwen::Pos::Fill);

	Gwen::Controls::Layout::TableRow* comboBoxRow = _layout->AddRow();

	_paneRow = _layout->AddRow();
	_paneRow->Dock(Gwen::Pos::Fill);

	_configSelectComboBox = new Gwen::Controls::ComboBox(comboBoxRow);
	_configSelectComboBox->Dock(Gwen::Pos::Fill);

	//_configSelectComboBox->SetPadding(Gwen::Padding(2, 2, 2, 2));
	//_configSelectComboBox->SetMargin(Gwen::Margin(2, 2, 2, 2));
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
	const type_info& ti = typeid( pChild );

	auto asBase = dynamic_cast<Gwen::Controls::Base*>(pChild);
	auto asTable = dynamic_cast<Gwen::Controls::Layout::Table*>(asBase);
	auto asPane = dynamic_cast<ConfigurationPaneBase*>(asTable);

	if (asPane)
	{
	}
}

void ConfigurationWindow::AddConfigPane(ConfigurationPaneBase* newPane)
{
	newPane->SetParent(_paneRow);

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

	//newPane->SetPadding(Gwen::Padding(2, 2, 2, 2));
	//newPane->SetMargin(Gwen::Margin(2, 2, 2, 2));
	//newPane->RestrictToParent(true);	
}

void ConfigurationWindow::RemoveConfigPane(ConfigurationPaneBase* removePane)
{
}

void ConfigurationWindow::OnFrameMove(double totalTime, float dt)
{
	std::map<Gwen::Controls::MenuItem*, ConfigurationPaneBase*>::iterator it;
	for (it = _paneMap.begin(); it != _paneMap.end(); it++)
	{
		it->second->OnFrameMove(totalTime, dt);
	}
}