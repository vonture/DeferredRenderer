#pragma once

#include "PCH.h"
#include "PostProcess.h"
#include "ConfigurationPaneBase.h"
#include "Gwen/Controls/ListBox.h"

class PostProcessSelectionPane : public ConfigurationPaneBase
{
private:
	struct PP_INFO
	{
		PostProcess* PP;
		Gwen::UnicodeString Name;
		bool CanMove;
	};

	std::vector<PP_INFO> _enabledPPs;
	std::vector<PP_INFO> _disabledPPs;

	Gwen::Controls::Label* _enabledLabel;
	Gwen::Controls::ListBox* _enabledPPListBox;

	Gwen::Controls::Label* _disabledLabel;
	Gwen::Controls::ListBox* _disabledPPListBox;

	Gwen::Controls::Button* _upButton;
	Gwen::Controls::Button* _downButton;

	Gwen::Controls::Button* _enableDisableButton;

	Gwen::Controls::ListBox* _lastSelectedListBox;

	int findIndexOfSelectedRow(Gwen::Controls::ListBox* box, std::vector<PP_INFO>* list);

	void syncListBoxes(int selectedIdx);
	
	virtual void onEnabledSelectionChanged(Gwen::Controls::Base* control);
	virtual void onDisabledSelectionChanged(Gwen::Controls::Base* control);

	virtual void onEnableDisableButtonPressed(Gwen::Controls::Base* control);

	virtual void onUpButtonPressed(Gwen::Controls::Base* control);
	virtual void onDownButtonPressed(Gwen::Controls::Base* control);

public:
	PostProcessSelectionPane(Gwen::Controls::Base* parent);

	void AddPostProcess(PostProcess* pp, const WCHAR* name, bool enabled = true, bool canMove = true);

	UINT GetSelectedPostProcessCount() const;
	PostProcess* GetSelectedPostProcesses(UINT idx) const; 

	bool IsPostProcessEnabled(PostProcess* pp) const;
	void SetPostProcessEnabled(PostProcess* pp, bool enabled);
};