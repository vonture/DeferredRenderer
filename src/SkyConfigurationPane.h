#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "SkyPostProcess.h"
#include "Gwen/Controls/ColorPicker.h"
#include "Gwen/Controls/CheckBox.h"
#include "Gwen/Controls/HorizontalSlider.h"
#include "DirectionSelector.h"

class SkyConfigurationPane : public ConfigurationPane<SkyPostProcess>
{
private:
	Gwen::Controls::Label* _skyColorLabel;
	Gwen::Controls::ColorPicker* _skyColorPicker;

	Gwen::Controls::CheckBoxWithLabel* _sunEnabledCheckBox;

	Gwen::Controls::Label* _sunColorLabel;
	Gwen::Controls::ColorPicker* _sunColorPicker;

	Gwen::Controls::Label* _sunIntensityLabel;
	Gwen::Controls::HorizontalSlider* _sunIntensitySlider;

	Gwen::Controls::Label* _sunWidthLabel;
	Gwen::Controls::HorizontalSlider* _sunWidthSlider;
	
	Gwen::Controls::Label* _sunDirLabel;
	DirectionSelector* _sunDirSelector;

	Gwen::Color vectorToGwenColor(const XMFLOAT3& vector);
	XMFLOAT3 gwenColorToVector(const Gwen::Color& color);
	
	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	SkyConfigurationPane(Gwen::Controls::Base* parent, SkyPostProcess* pp);
	~SkyConfigurationPane();

	void OnFrameMove(double totalTime, float dt);
};