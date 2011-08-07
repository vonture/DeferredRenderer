#include "PCH.h"
#include "SSAOConfigurationPane.h"

SSAOConfigurationPane::SSAOConfigurationPane(Gwen::Controls::Base* parent, SSAOPostProcess* pp)
	: ConfigurationPane(parent, L"SSAO", pp)
{
	const int childWidth = 240;
	const int labelHight = 20;
	const int sliderHeight = 14;
	const int spacing = 3;

	_sampleCountLabel = new Gwen::Controls::Label(this);
	_sampleCountLabel->SetText("");
	_sampleCountLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_sampleCountLabel->SetBounds(0, 0, childWidth, labelHight);

	_sampleCountSlider = new Gwen::Controls::HorizontalSlider(this);
	_sampleCountSlider->SetClampToNotches(true);
	_sampleCountSlider->SetRange(0.0f, 4.0f);
	_sampleCountSlider->SetNotchCount(5);
	_sampleCountSlider->SetValue(pp->GetSampleCountIndex());
	_sampleCountSlider->SetBounds(0, _sampleCountLabel->Bottom(), childWidth, sliderHeight);
	_sampleCountSlider->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);

	_sampleRadiusLabel = new Gwen::Controls::Label(this);
	_sampleRadiusLabel->SetText("");
	_sampleRadiusLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_sampleRadiusLabel->SetBounds(0, _sampleCountSlider->Bottom() + spacing, childWidth, labelHight);

	_sampleRadiusSlider = new Gwen::Controls::HorizontalSlider(this);
	_sampleRadiusSlider->SetClampToNotches(false);
	_sampleRadiusSlider->SetRange(0.0f, 5.0f);
	_sampleRadiusSlider->SetValue(pp->GetSampleRadius());
	_sampleRadiusSlider->SetBounds(0, _sampleRadiusLabel->Bottom(), childWidth, sliderHeight);
	_sampleRadiusSlider->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);

	_samplePowerLabel = new Gwen::Controls::Label(this);
	_samplePowerLabel->SetText("");
	_samplePowerLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_samplePowerLabel->SetBounds(0, _sampleRadiusSlider->Bottom() + spacing, childWidth, labelHight);

	_samplePowerSlider = new Gwen::Controls::HorizontalSlider(this);
	_samplePowerSlider->SetClampToNotches(false);
	_samplePowerSlider->SetRange(EPSILON, 5.0f);
	_samplePowerSlider->SetValue(pp->GetSamplePower());
	_samplePowerSlider->SetBounds(0, _samplePowerLabel->Bottom(), childWidth, sliderHeight);
	_samplePowerSlider->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);

	_blurSigmaLabel = new Gwen::Controls::Label(this);
	_blurSigmaLabel->SetText("");
	_blurSigmaLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_blurSigmaLabel->SetBounds(0, _samplePowerSlider->Bottom() + spacing, childWidth, labelHight);

	_blurSigmaSlider = new Gwen::Controls::HorizontalSlider(this);
	_blurSigmaSlider->SetClampToNotches(false);
	_blurSigmaSlider->SetRange(EPSILON, 4.0f);
	_blurSigmaSlider->SetValue(pp->GetBlurSigma());
	_blurSigmaSlider->SetBounds(0, _blurSigmaLabel->Bottom(), childWidth, sliderHeight);
	_blurSigmaSlider->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);
}

void SSAOConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
	SSAOPostProcess* pp = GetConfiguredObject();

	if (control == _sampleCountSlider)
	{
		pp->SetSampleCountIndex(floor(_sampleCountSlider->GetValue() + 0.5f));
	}
	else if (control == _sampleRadiusSlider)
	{
		pp->SetSampleRadius(_sampleRadiusSlider->GetValue());
	}
	else if (control == _samplePowerSlider)
	{
		pp->SetSamplePower(_samplePowerSlider->GetValue());
	}
	else if (control == _blurSigmaSlider)
	{
		pp->SetBlurSigma(_blurSigmaSlider->GetValue());
	}
}

void SSAOConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	SSAOPostProcess* pp = GetConfiguredObject();

	_sampleCountSlider->SetValue(pp->GetSampleCountIndex());
	_sampleCountLabel->SetText("Sample count: " + Gwen::Utility::ToString(pp->GetSampleCount())); 

	_sampleRadiusSlider->SetValue(pp->GetSampleRadius());
	_sampleRadiusLabel->SetText("Sample radius: " + Gwen::Utility::ToString(pp->GetSampleRadius())); 

	_samplePowerSlider->SetValue(pp->GetSamplePower());
	_samplePowerLabel->SetText("Sample power: " + Gwen::Utility::ToString(pp->GetSamplePower())); 

	_blurSigmaSlider->SetValue(pp->GetBlurSigma());
	_blurSigmaLabel->SetText("Blur sigma: " + Gwen::Utility::ToString(pp->GetBlurSigma())); 
}