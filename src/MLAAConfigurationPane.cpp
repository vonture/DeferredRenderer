#include "PCH.h"
#include "MLAAConfigurationPane.h"

MLAAConfigurationPane::MLAAConfigurationPane(Gwen::Controls::Base* parent, MLAAPostProcess* pp)
	: ConfigurationPane(parent, L"MLAA", pp), _depthThresholdSlider(NULL), _depthThresholdLabel(NULL), 
	  _normalThresholdSlider(NULL), _normalThresholdLabel(NULL), _lumThresholdSlider(NULL), _lumThresholdLabel(NULL),
	  _maxSearchStepsSlider(NULL), _maxSearchStepsLabel(NULL), _depthDetectionCheckBox(NULL), _normalDetectionCheckBox(NULL),
	  _lumDetectionCheckBox(NULL)
{
	const int childWidth = 240;
	const int labelHight = 20;
	const int sliderHeight = 14;
	const int spacing = 3;

	// Depth settings
	_depthDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_depthDetectionCheckBox->Label()->SetText("Depth detection enabled");
	_depthDetectionCheckBox->Checkbox()->SetChecked(pp->GetDepthDetectionEnabled());
	_depthDetectionCheckBox->SetBounds(0, 0, childWidth, labelHight);
	_depthDetectionCheckBox->Checkbox()->onCheckChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

	_depthThresholdLabel = new Gwen::Controls::Label(this);
	_depthThresholdLabel->SetText("");
	_depthThresholdLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_depthThresholdLabel->SetBounds(0, _depthDetectionCheckBox->Bottom(), childWidth, labelHight);

	_depthThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_depthThresholdSlider->SetClampToNotches(false);
	_depthThresholdSlider->SetRange(EPSILON, 2.0f);
	_depthThresholdSlider->SetValue(pp->GetDepthThreshold());
	_depthThresholdSlider->SetBounds(0, _depthThresholdLabel->Bottom(), childWidth, sliderHeight);
	_depthThresholdSlider->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

	// Normal settings
	_normalDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_normalDetectionCheckBox->Label()->SetText("Normal detection enabled");
	_normalDetectionCheckBox->Checkbox()->SetChecked(pp->GetNormalDetectionEnabled());
	_normalDetectionCheckBox->SetBounds(0, _depthThresholdSlider->Bottom() + spacing, childWidth, labelHight);
	_normalDetectionCheckBox->Checkbox()->onCheckChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

	_normalThresholdLabel = new Gwen::Controls::Label(this);
	_normalThresholdLabel->SetText("");
	_normalThresholdLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_normalThresholdLabel->SetBounds(0, _normalDetectionCheckBox->Bottom(), childWidth, labelHight);

	_normalThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_normalThresholdSlider->SetClampToNotches(false);
	_normalThresholdSlider->SetRange(EPSILON, 0.5f);
	_normalThresholdSlider->SetValue(pp->GetNormalThreshold());
	_normalThresholdSlider->SetBounds(0, _normalThresholdLabel->Bottom(), childWidth, sliderHeight);
	_normalThresholdSlider->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

	// Luminance settings
	_lumDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_lumDetectionCheckBox->Label()->SetText("Luminance detection enabled");
	_lumDetectionCheckBox->Checkbox()->SetChecked(pp->GetLuminanceDetectionEnabled());
	_lumDetectionCheckBox->SetBounds(0, _normalThresholdSlider->Bottom() + spacing, childWidth, labelHight);
	_lumDetectionCheckBox->Checkbox()->onCheckChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

	_lumThresholdLabel = new Gwen::Controls::Label(this);
	_lumThresholdLabel->SetText("");
	_lumThresholdLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_lumThresholdLabel->SetBounds(0, _lumDetectionCheckBox->Bottom(), childWidth, labelHight);

	_lumThresholdSlider = new Gwen::Controls::HorizontalSlider(this);
	_lumThresholdSlider->SetClampToNotches(false);
	_lumThresholdSlider->SetRange(EPSILON, 0.5f);
	_lumThresholdSlider->SetValue(pp->GetLuminanceThreshold());	
	_lumThresholdSlider->SetBounds(0, _lumThresholdLabel->Bottom(), childWidth, sliderHeight);
	_lumThresholdSlider->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

	// Search stesps setting
	_maxSearchStepsLabel = new Gwen::Controls::Label(this);
	_maxSearchStepsLabel->SetText("");
	_maxSearchStepsLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_maxSearchStepsLabel->SetBounds(0, _lumThresholdSlider->Bottom() + spacing, childWidth, labelHight);

	_maxSearchStepsSlider = new Gwen::Controls::HorizontalSlider(this);
	_maxSearchStepsSlider->SetClampToNotches(true);
	_maxSearchStepsSlider->SetRange(4.0f, 64.0f);
	_maxSearchStepsSlider->SetNotchCount(64 - 4 + 1);
	_maxSearchStepsSlider->SetValue(pp->GetMaxSearchSteps());
	_maxSearchStepsSlider->SetBounds(0, _maxSearchStepsLabel->Bottom() + spacing, childWidth, sliderHeight);
	_maxSearchStepsSlider->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);
}

MLAAConfigurationPane::~MLAAConfigurationPane()
{
}

void MLAAConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
	MLAAPostProcess* pp = GetConfiguredObject();

	if (control == _depthDetectionCheckBox->Checkbox())
	{
		pp->SetDepthDetectionEnabled(_depthDetectionCheckBox->Checkbox()->IsChecked());
	}
	else if (control == _depthThresholdSlider)
	{
		pp->SetDepthThreshold(_depthThresholdSlider->GetValue());
	}
	else if (control == _normalDetectionCheckBox->Checkbox())
	{
		pp->SetNormalDetectionEnabled(_normalDetectionCheckBox->Checkbox()->IsChecked());
	}
	else if (control == _normalThresholdSlider)
	{
		pp->SetNormalThreshold(_normalThresholdSlider->GetValue());
	}
	else if (control == _lumDetectionCheckBox->Checkbox())
	{
		pp->SetLuminanceDetectionEnabled(_lumDetectionCheckBox->Checkbox()->IsChecked());
	}
	else if (control == _lumThresholdSlider)
	{
		pp->SetLuminanceThreshold(_lumThresholdSlider->GetValue());
	}
	else if (control == _maxSearchStepsSlider)
	{
		pp->SetMaxSearchSteps(floor(_maxSearchStepsSlider->GetValue() + 0.5f));
	}
}

void MLAAConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	MLAAPostProcess* pp = GetConfiguredObject();

	_depthDetectionCheckBox->Checkbox()->SetChecked(pp->GetDepthDetectionEnabled());

	_depthThresholdSlider->SetValue(pp->GetDepthThreshold());
	_depthThresholdLabel->SetText("Depth threshold: " + Gwen::Utility::ToString(pp->GetDepthThreshold()));

	_normalDetectionCheckBox->Checkbox()->SetChecked(pp->GetNormalDetectionEnabled());

	_normalThresholdSlider->SetValue(pp->GetNormalThreshold());
	_normalThresholdLabel->SetText("Normal threshold: " + Gwen::Utility::ToString(pp->GetNormalThreshold()));

	_lumDetectionCheckBox->Checkbox()->SetChecked(pp->GetLuminanceDetectionEnabled());

	_lumThresholdSlider->SetValue(pp->GetLuminanceThreshold());
	_lumThresholdLabel->SetText("Luminance threshold: " + Gwen::Utility::ToString(pp->GetLuminanceThreshold()));

	_maxSearchStepsSlider->SetValue(pp->GetMaxSearchSteps());
	_maxSearchStepsLabel->SetText("Max search steps: " + Gwen::Utility::ToString(pp->GetMaxSearchSteps()));
}