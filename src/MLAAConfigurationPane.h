#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "MLAAPostProcess.h"
#include "Gwen/Controls/HorizontalSlider.h"
#include "Gwen/Controls/CheckBox.h"

class MLAAConfigurationPane : public ConfigurationPane<MLAAPostProcess>
{
private:
	Gwen::Controls::CheckBoxWithLabel* _depthDetectionCheckBox;
	Gwen::Controls::HorizontalSlider* _depthThresholdSlider;
	Gwen::Controls::Label* _depthThresholdLabel;

	Gwen::Controls::CheckBoxWithLabel* _normalDetectionCheckBox;
	Gwen::Controls::HorizontalSlider* _normalThresholdSlider;
	Gwen::Controls::Label* _normalThresholdLabel;

	Gwen::Controls::HorizontalSlider* _lumThresholdSlider;
	Gwen::Controls::Label* _lumThresholdLabel;
	Gwen::Controls::CheckBoxWithLabel* _lumDetectionCheckBox;

	Gwen::Controls::HorizontalSlider* _maxSearchStepsSlider;
	Gwen::Controls::Label* _maxSearchStepsLabel;

	virtual void OnValueChanged(Gwen::Controls::Base *control);

public:
	MLAAConfigurationPane(Gwen::Controls::Base* parent, MLAAPostProcess* pp);
	~MLAAConfigurationPane();

	void OnFrameMove(double totalTime, float dt);
};