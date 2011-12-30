#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "BoundingObjectPostProcess.h"
#include "Gwen/Controls/ColorPicker.h"
#include "Gwen/Controls/CheckBox.h"

class BoundingObjectConfigurationPane : public ConfigurationPane<BoundingObjectPostProcess>
{
private:
	Gwen::Controls::CheckBoxWithLabel* _lightEnableCheckBox;
	Gwen::Controls::CheckBoxWithLabel* _modelEnableCheckBox;
	Gwen::Controls::CheckBoxWithLabel* _particleEnableCheckBox;

	Gwen::Controls::Label* _boColorLabel;
	Gwen::Controls::ColorPicker* _boColorPicker;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	BoundingObjectConfigurationPane(Gwen::Controls::Base* parent, BoundingObjectPostProcess* pp);
	
	void SetParticlesEnabled(bool val) { _particleEnableCheckBox->Checkbox()->SetChecked(val); }
	void SetModelsEnabled(bool val) { _modelEnableCheckBox->Checkbox()->SetChecked(val); }
	void SetLightsEnabled(bool val) { _lightEnableCheckBox->Checkbox()->SetChecked(val); }

	bool GetParticlesEnabled() const { return _particleEnableCheckBox->Checkbox()->IsChecked(); }
	bool GetModelsEnabled() const { return _modelEnableCheckBox->Checkbox()->IsChecked(); }
	bool GetLightsEnabled() const { return _lightEnableCheckBox->Checkbox()->IsChecked(); }

	void OnFrameMove(double totalTime, float dt);
};