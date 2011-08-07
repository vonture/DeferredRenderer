#include "SkyConfigurationPane.h"

SkyConfigurationPane::SkyConfigurationPane(Gwen::Controls::Base* parent, SkyPostProcess* pp)
	: ConfigurationPane(parent, L"Sky", pp)
{
	const int childWidth = 240;
	const int labelHight = 20;
	const int sliderHeight = 14;
	const int colorPickerHeight = 150;
	const int directionSelectorHeight = 150;
	const int spacing = 3;

	_skyColorLabel = new Gwen::Controls::Label(this);
	_skyColorLabel->SetText("Sky color:");
	_skyColorLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_skyColorLabel->SetBounds(0, 0, childWidth, labelHight);
	
	_skyColorPicker = new Gwen::Controls::ColorPicker(this);
	_skyColorPicker->SetColor(vectorToGwenColor(pp->GetSkyColor()));
	_skyColorPicker->SetBounds(0, _skyColorLabel->Bottom(), childWidth, colorPickerHeight);
	_skyColorPicker->onColorChanged.Add(this, &SkyConfigurationPane::OnValueChanged);

	_sunEnabledCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
	_sunEnabledCheckBox->Label()->SetText("Sun enabled");
	_sunEnabledCheckBox->Checkbox()->SetChecked(pp->GetSunEnabled());
	_sunEnabledCheckBox->SetBounds(0, _skyColorPicker->Bottom() + spacing, childWidth, labelHight);
	_sunEnabledCheckBox->Checkbox()->onCheckChanged.Add(this, &SkyConfigurationPane::OnValueChanged);

	_sunColorLabel = new Gwen::Controls::Label(this);
	_sunColorLabel->SetText("Sun color:");
	_sunColorLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_sunColorLabel->SetBounds(0, _sunEnabledCheckBox->Bottom() + spacing, childWidth, labelHight);
	
	_sunColorPicker = new Gwen::Controls::ColorPicker(this);
	_sunColorPicker->SetColor(vectorToGwenColor(pp->GetSunColor()));
	_sunColorPicker->SetBounds(0, _sunColorLabel->Bottom(), childWidth, colorPickerHeight);
	_sunColorPicker->onColorChanged.Add(this, &SkyConfigurationPane::OnValueChanged);
	
	_sunIntensityLabel = new Gwen::Controls::Label(this);
	_sunIntensityLabel->SetText("");
	_sunIntensityLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_sunIntensityLabel->SetBounds(0, _sunColorPicker->Bottom(), childWidth, labelHight);

	_sunIntensitySlider = new Gwen::Controls::HorizontalSlider(this);
	_sunIntensitySlider->SetClampToNotches(false);
	_sunIntensitySlider->SetRange(0.0f, 25.0f);
	_sunIntensitySlider->SetValue(pp->GetSunIntensity());	
	_sunIntensitySlider->SetBounds(0, _sunIntensityLabel->Bottom(), childWidth, sliderHeight);
	_sunIntensitySlider->onValueChanged.Add(this, &SkyConfigurationPane::OnValueChanged);

	_sunDirLabel = new Gwen::Controls::Label(this);
	_sunDirLabel->SetText("");
	_sunDirLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_sunDirLabel->SetBounds(0, _sunIntensitySlider->Bottom(), childWidth, labelHight);

	_sunDirSelector = new DirectionSelector(this);
	XMFLOAT3 sunDir = pp->GetSunDirection();
	_sunDirSelector->SetDirection(XMFLOAT2(sunDir.x, sunDir.z));
	_sunDirSelector->SetBounds(0, _sunDirLabel->Bottom(), directionSelectorHeight, directionSelectorHeight);
	_sunDirSelector->onValueChanged.Add(this, &SkyConfigurationPane::OnValueChanged);

	_sunWidthLabel = new Gwen::Controls::Label(this);
	_sunWidthLabel->SetText("");
	_sunWidthLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_sunWidthLabel->SetBounds(0, _sunDirSelector->Bottom(), childWidth, labelHight);

	_sunWidthSlider = new Gwen::Controls::HorizontalSlider(this);
	_sunWidthSlider->SetClampToNotches(false);
	_sunWidthSlider->SetRange(EPSILON, 0.5f);
	_sunWidthSlider->SetValue(pp->GetSunWidth());	
	_sunWidthSlider->SetBounds(0, _sunWidthLabel->Bottom(), childWidth, sliderHeight);
	_sunWidthSlider->onValueChanged.Add(this, &SkyConfigurationPane::OnValueChanged);
}

SkyConfigurationPane::~SkyConfigurationPane()
{
}

Gwen::Color SkyConfigurationPane::vectorToGwenColor(const XMFLOAT3& vector)
{
	return Gwen::Color(vector.x * 255, vector.y * 255, vector.z * 255);
}

XMFLOAT3 SkyConfigurationPane::gwenColorToVector(const Gwen::Color& color)
{
	return XMFLOAT3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
}

void SkyConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
	SkyPostProcess* pp = GetConfiguredObject();

	if (control == _skyColorPicker)
	{
		pp->SetSkyColor(gwenColorToVector(_skyColorPicker->GetColor()));
	}
	else if (control == _sunEnabledCheckBox->Checkbox())
	{
		pp->SetSunEnabled(_sunEnabledCheckBox->Checkbox()->IsChecked());
	}
	else if (control == _sunColorPicker)
	{
		pp->SetSunColor(gwenColorToVector(_sunColorPicker->GetColor()));
	}
	else if (control == _sunWidthSlider)
	{
		pp->SetSunWidth(_sunWidthSlider->GetValue());
	}
	else if (control == _sunIntensitySlider)
	{
		pp->SetSunIntensity(_sunIntensitySlider->GetValue());
	}
	else if (control == _sunDirSelector)
	{
		XMFLOAT2 dir = _sunDirSelector->GetDirection();
		float y = sqrtf(1.0f - (dir.x * dir.x + dir.y * dir.y));
		pp->SetSunDirection(XMFLOAT3(dir.x, y, dir.y));
	}
}

void SkyConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	SkyPostProcess* pp = GetConfiguredObject();

	_skyColorPicker->SetColor(vectorToGwenColor(pp->GetSkyColor()));
	_sunEnabledCheckBox->Checkbox()->SetChecked(pp->GetSunEnabled());
	_sunColorPicker->SetColor(vectorToGwenColor(pp->GetSunColor()));

	_sunIntensitySlider->SetValue(pp->GetSunIntensity());
	_sunIntensityLabel->SetText("Sun Intensity: " + Gwen::Utility::ToString(pp->GetSunIntensity()));

	_sunWidthSlider->SetValue(pp->GetSunWidth());
	_sunWidthLabel->SetText("Sun width: " + Gwen::Utility::ToString(pp->GetSunWidth()));

	XMFLOAT3 sunDir = pp->GetSunDirection();
	_sunDirSelector->SetDirection(XMFLOAT2(sunDir.x, sunDir.z));
	_sunDirLabel->SetText("Sun direction: (" + Gwen::Utility::ToString(sunDir.x) + 
		", " + Gwen::Utility::ToString(sunDir.y) + ", " + Gwen::Utility::ToString(sunDir.z) + ")");
}