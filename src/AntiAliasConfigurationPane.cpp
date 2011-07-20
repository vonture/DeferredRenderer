#include "AntiAliasConfigurationPane.h"

AntiAliasConfigurationPane::AntiAliasConfigurationPane(Gwen::Controls::Base* parent, AntiAliasPostProcess* pp)
	: ConfigurationPane(parent, pp), _depthThresholdSlider(NULL), _depthThresholdLabel(NULL), 
	  _normalThresholdSlider(NULL), _normalThresholdLabel(NULL), _lumThresholdSlider(NULL), _lumThresholdLabel(NULL),
	  _maxSearchStepsSlider(NULL), _maxSearchStepsLabel(NULL), _depthDetectionCheckBox(NULL), _normalDetectionCheckBox(NULL),
	  _lumDetectionCheckBox(NULL)
{
	SetName("Anti-Alias Post Process");

	// Depth settings
	_depthDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_depthDetectionCheckBox->SetHeight(20);
	_depthDetectionCheckBox->Label()->SetText("Depth detection enabled");
	_depthDetectionCheckBox->Checkbox()->SetChecked(pp->GetDepthDetectionEnabled());
	_depthDetectionCheckBox->Dock(Gwen::Pos::Top);

	_depthThresholdLabel = new Gwen::Controls::Label(this);
	_depthThresholdLabel->SetHeight(16);
	_depthThresholdLabel->SetText("");
	_depthThresholdLabel->Dock(Gwen::Pos::Top);

	_depthThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_depthThresholdSlider->SetClampToNotches(false);
	_depthThresholdSlider->SetRange(0.0001f, 2.0f);
	_depthThresholdSlider->SetValue(pp->GetDepthThreshold());
	_depthThresholdSlider->SetHeight(16);
	_depthThresholdSlider->Dock(Gwen::Pos::Top);

	// Normal settings
	_normalDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_normalDetectionCheckBox->SetHeight(20);
	_normalDetectionCheckBox->Label()->SetText("Normal detection enabled");
	_normalDetectionCheckBox->Checkbox()->SetChecked(pp->GetNormalDetectionEnabled());
	_normalDetectionCheckBox->Dock(Gwen::Pos::Top);

	_normalThresholdLabel = new Gwen::Controls::Label(this);
	_normalThresholdLabel->SetHeight(16);
	_normalThresholdLabel->SetText("");
	_normalThresholdLabel->Dock(Gwen::Pos::Top);

	_normalThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_normalThresholdSlider->SetClampToNotches(false);
	_normalThresholdSlider->SetRange(0.0001f, 0.5f);
	_normalThresholdSlider->SetValue(pp->GetNormalThreshold());
	_normalThresholdSlider->SetHeight(16);
	_normalThresholdSlider->Dock(Gwen::Pos::Top);

	// Luminance settings
	_lumDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_lumDetectionCheckBox->SetHeight(20);
	_lumDetectionCheckBox->Label()->SetText("Luminance detection enabled");
	_lumDetectionCheckBox->Checkbox()->SetChecked(pp->GetLuminanceDetectionEnabled());
	_lumDetectionCheckBox->Dock(Gwen::Pos::Top);

	_lumThresholdLabel = new Gwen::Controls::Label(this);
	_lumThresholdLabel->SetHeight(16);
	_lumThresholdLabel->SetText("");
	_lumThresholdLabel->Dock(Gwen::Pos::Top);

	_lumThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_lumThresholdSlider->SetClampToNotches(false);
	_lumThresholdSlider->SetRange(0.0001f, 0.5f);
	_lumThresholdSlider->SetValue(pp->GetLuminanceThreshold());
	_lumThresholdSlider->SetHeight(16);
	_lumThresholdSlider->Dock(Gwen::Pos::Top);

	// Search stesps setting
	_maxSearchStepsLabel = new Gwen::Controls::Label(this);
	_maxSearchStepsLabel->SetHeight(16);
	_maxSearchStepsLabel->SetText("");
	_maxSearchStepsLabel->Dock(Gwen::Pos::Top);

	_maxSearchStepsSlider = new Gwen::Controls::HorizontalSlider(this);
	_maxSearchStepsSlider->SetClampToNotches(true);
	_maxSearchStepsSlider->SetRange(4.0f, 64.0f);
	_maxSearchStepsSlider->SetNotchCount(64 - 4 + 1);
	_maxSearchStepsSlider->SetValue(pp->GetMaxSearchSteps());
	_maxSearchStepsSlider->SetHeight(16);
	_maxSearchStepsSlider->Dock(Gwen::Pos::Top);
}

AntiAliasConfigurationPane::~AntiAliasConfigurationPane()
{
}

void AntiAliasConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	AntiAliasPostProcess* pp = GetConfiguredObject();

	pp->SetDepthDetectionEnabled(_depthDetectionCheckBox->Checkbox()->IsChecked());
	_depthDetectionCheckBox->Checkbox()->SetChecked(pp->GetDepthDetectionEnabled());

	pp->SetDepthThreshold(_depthThresholdSlider->GetValue());
	_depthThresholdSlider->SetValue(pp->GetDepthThreshold());
	_depthThresholdLabel->SetText("Depth threshold: " + Gwen::Utility::ToString(pp->GetDepthThreshold()));


	pp->SetNormalDetectionEnabled(_normalDetectionCheckBox->Checkbox()->IsChecked());
	_normalDetectionCheckBox->Checkbox()->SetChecked(pp->GetNormalDetectionEnabled());

	pp->SetNormalThreshold(_normalThresholdSlider->GetValue());
	_normalThresholdSlider->SetValue(pp->GetNormalThreshold());
	_normalThresholdLabel->SetText("Normal threshold: " + Gwen::Utility::ToString(pp->GetNormalThreshold()));


	pp->SetLuminanceDetectionEnabled(_lumDetectionCheckBox->Checkbox()->IsChecked());
	_lumDetectionCheckBox->Checkbox()->SetChecked(pp->GetLuminanceDetectionEnabled());

	pp->SetLuminanceThreshold(_lumThresholdSlider->GetValue());
	_lumThresholdSlider->SetValue(pp->GetLuminanceThreshold());
	_lumThresholdLabel->SetText("Luminance threshold: " + Gwen::Utility::ToString(pp->GetLuminanceThreshold()));

	pp->SetMaxSearchSteps(floor(_maxSearchStepsSlider->GetValue() + 0.5f));
	_maxSearchStepsSlider->SetValue(pp->GetMaxSearchSteps());
	_maxSearchStepsLabel->SetText("Max search steps: " + Gwen::Utility::ToString(pp->GetMaxSearchSteps()));
}