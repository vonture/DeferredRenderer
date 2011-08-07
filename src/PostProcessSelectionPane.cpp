#include "PCH.h"
#include "PostProcessSelectionPane.h"

PostProcessSelectionPane::PostProcessSelectionPane(Gwen::Controls::Base* parent)
	: ConfigurationPaneBase(parent, L"Post Processes")
{	
	const int childWidth = 240;
	const int labelHight = 20;
	const int listBoxHeight = 200;
	const int buttonHeight = 30;
	const int spacing = 3;

	_enabledLabel = new Gwen::Controls::Label(this);
	_enabledLabel->SetText("Enabled:");
	_enabledLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_enabledLabel->SetBounds(0, 0, childWidth, labelHight);

	_enabledPPListBox = new Gwen::Controls::ListBox(this);
	_enabledPPListBox->SetBounds(0, _enabledLabel->Bottom(), childWidth, listBoxHeight);
	_enabledPPListBox->onRowSelected.Add(this, &PostProcessSelectionPane::onEnabledSelectionChanged);

	_disabledLabel = new Gwen::Controls::Label(this);
	_disabledLabel->SetText("Disabled:");
	_disabledLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_disabledLabel->SetBounds(0, _enabledPPListBox->Bottom() + spacing, childWidth, labelHight);

	_disabledPPListBox = new Gwen::Controls::ListBox(this);
	_disabledPPListBox->SetBounds(0, _disabledLabel->Bottom(), childWidth, listBoxHeight);
	_disabledPPListBox->onRowSelected.Add(this, &PostProcessSelectionPane::onDisabledSelectionChanged);

	_enableDisableButton = new Gwen::Controls::Button(this);
	_enableDisableButton->SetText("Enable/Disable");
	_enableDisableButton->SetBounds(0, _disabledPPListBox->Bottom() + spacing, childWidth, buttonHeight);
	_enableDisableButton->onPress.Add(this, &PostProcessSelectionPane::onEnableDisableButtonPressed);

	_upButton = new Gwen::Controls::Button(this);
	_upButton->SetText("Move up");
	_upButton->SetBounds(0, _enableDisableButton->Bottom() + spacing, childWidth, buttonHeight);
	_upButton->onPress.Add(this, &PostProcessSelectionPane::onUpButtonPressed);

	_downButton = new Gwen::Controls::Button(this);
	_downButton->SetText("Move down");
	_downButton->SetBounds(0, _upButton->Bottom(), childWidth, buttonHeight);
	_downButton->onPress.Add(this, &PostProcessSelectionPane::onDownButtonPressed);

	_lastSelectedListBox = _enabledPPListBox;
}

void PostProcessSelectionPane::syncListBoxes(int selectedIdx)
{
	_enabledPPListBox->Clear();
	_disabledPPListBox->Clear();

	Gwen::Controls::Layout::TableRow* selectedRow = NULL;

	for (UINT i = 0; i < _enabledPPs.size(); i++)
	{
		Gwen::Controls::Layout::TableRow* row = _enabledPPListBox->AddItem(_enabledPPs[i].Name);

		if (_lastSelectedListBox == _enabledPPListBox && (int)i == selectedIdx)
		{
			selectedRow = row;
		}
	}
	for (UINT i = 0; i < _disabledPPs.size(); i++)
	{
		Gwen::Controls::Layout::TableRow* row = _disabledPPListBox->AddItem(_disabledPPs[i].Name);

		if (_lastSelectedListBox == _disabledPPListBox && (int)i == selectedIdx)
		{
			selectedRow = row;			
		}
	}

	if (selectedRow)
	{
		// Terrible solution, simulate a click on this item so that it becomes selected
		selectedRow->OnMouseClickLeft(0, 0, true);
		selectedRow->OnMouseClickLeft(0, 0, false);
	}
}

int PostProcessSelectionPane::findIndexOfSelectedRow(Gwen::Controls::ListBox* box, std::vector<PP_INFO>* list)
{
	Gwen::Controls::Layout::TableRow* row = box->GetSelectedRow();
	if (row)
	{
		for (UINT i = 0; i < list->size(); i++)
		{
			if (list->at(i).Name == row->GetText(0))
			{
				return i;
			}
		}
	}

	return -1;
}

void PostProcessSelectionPane::onEnabledSelectionChanged(Gwen::Controls::Base* control)
{
	_lastSelectedListBox = _enabledPPListBox;
	_disabledPPListBox->UnselectAll();
}

void PostProcessSelectionPane::onDisabledSelectionChanged(Gwen::Controls::Base* control)
{
	_lastSelectedListBox = _disabledPPListBox;
	_enabledPPListBox->UnselectAll();
}

void PostProcessSelectionPane::onEnableDisableButtonPressed(Gwen::Controls::Base* control)
{
	if (!_lastSelectedListBox)
	{
		return;
	}

	std::vector<PP_INFO>* from = (_lastSelectedListBox == _enabledPPListBox) ? &_enabledPPs : &_disabledPPs;
	std::vector<PP_INFO>* to = (_lastSelectedListBox == _enabledPPListBox) ? &_disabledPPs : &_enabledPPs;
	
	int selectedIdx = findIndexOfSelectedRow(_lastSelectedListBox, from);
	if (selectedIdx >= 0)
	{
		if (!from->at(selectedIdx).CanMove)
		{
			return;
		}

		to->push_back(from->at(selectedIdx));
		from->erase(from->begin() + selectedIdx);

		_lastSelectedListBox = (_lastSelectedListBox == _enabledPPListBox) ? _disabledPPListBox : _enabledPPListBox;
		syncListBoxes(to->size() - 1);
	}
}

void PostProcessSelectionPane::onUpButtonPressed(Gwen::Controls::Base* control)
{
	if (_lastSelectedListBox != _enabledPPListBox)
	{
		return;
	}

	int selectedIdx = findIndexOfSelectedRow(_enabledPPListBox, &_enabledPPs);
	if (selectedIdx > 0)
	{
		PP_INFO temp = _enabledPPs[selectedIdx - 1];
		_enabledPPs[selectedIdx - 1] = _enabledPPs[selectedIdx];
		_enabledPPs[selectedIdx] = temp;

		syncListBoxes(selectedIdx - 1);
	}
}

void PostProcessSelectionPane::onDownButtonPressed(Gwen::Controls::Base* control)
{
	if (_lastSelectedListBox != _enabledPPListBox)
	{
		return;
	}

	int selectedIdx = findIndexOfSelectedRow(_enabledPPListBox, &_enabledPPs);
	if (selectedIdx >= 0 && (UINT)selectedIdx + 1 < _enabledPPs.size())
	{
		PP_INFO temp = _enabledPPs[selectedIdx + 1];
		_enabledPPs[selectedIdx + 1] = _enabledPPs[selectedIdx];
		_enabledPPs[selectedIdx] = temp;

		syncListBoxes(selectedIdx + 1);
	}
}

void PostProcessSelectionPane::AddPostProcess(PostProcess* pp, const WCHAR* name, bool enabled,
	bool canMove)
{
	PP_INFO info;
	info.PP = pp;
	info.Name = Gwen::UnicodeString(name);
	info.CanMove = canMove;

	if (enabled)
	{
		_enabledPPs.push_back(info);
	}
	else
	{
		_disabledPPs.push_back(info);
	}

	syncListBoxes(-1);
}

UINT PostProcessSelectionPane::GetSelectedPostProcessCount() const
{
	return _enabledPPs.size();
}

PostProcess* PostProcessSelectionPane::GetSelectedPostProcesses(UINT idx) const
{
	return _enabledPPs[idx].PP;
}

bool PostProcessSelectionPane::IsPostProcessEnabled(PostProcess* pp) const
{
	for (UINT i = 0; i < _enabledPPs.size(); i++)
	{
		if (_enabledPPs[i].PP == pp)
		{
			return true;
		}
	}

	return false;
}

void PostProcessSelectionPane::SetPostProcessEnabled(PostProcess* pp, bool enabled)
{
	std::vector<PP_INFO>* from = enabled ? &_disabledPPs : &_enabledPPs;
	std::vector<PP_INFO>* to = enabled ? &_enabledPPs : &_disabledPPs;

	for (UINT i = 0; i < from->size(); i++)
	{
		if (from->at(i).PP == pp)
		{
			to->push_back(from->at(i));
			from->erase(from->begin() + i);
			break;
		}
	}
}