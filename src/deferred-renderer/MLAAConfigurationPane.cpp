#include "PCH.h"
#include "MLAAConfigurationPane.h"

MLAAConfigurationPane::MLAAConfigurationPane(Gwen::Controls::Base* parent, MLAAPostProcess* pp)
    : ConfigurationPane(parent, L"MLAA", pp)
{
    // Depth settings
    _depthDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _depthDetectionCheckBox->Label()->SetText("Depth detection enabled");
    _depthDetectionCheckBox->Checkbox()->SetChecked(pp->GetDepthDetectionEnabled());
    _depthDetectionCheckBox->Dock(Gwen::Pos::Top);
    _depthDetectionCheckBox->Checkbox()->onCheckChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

    _depthThresholdSlider = new SliderWithLabel(this);
    _depthThresholdSlider->Slider()->SetRange(EPSILON, 2.0f);
    _depthThresholdSlider->Slider()->SetValue(pp->GetDepthThreshold());
    _depthThresholdSlider->Slider()->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);
    _depthThresholdSlider->Dock(Gwen::Pos::Top);

    // Normal settings
    _normalDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _normalDetectionCheckBox->Label()->SetText("Normal detection enabled");
    _normalDetectionCheckBox->Checkbox()->SetChecked(pp->GetNormalDetectionEnabled());
    _normalDetectionCheckBox->Dock(Gwen::Pos::Top);
    _normalDetectionCheckBox->Checkbox()->onCheckChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

    _normalThresholdSlider = new SliderWithLabel(this);
    _normalThresholdSlider->Slider()->SetClampToNotches(false);
    _normalThresholdSlider->Slider()->SetRange(EPSILON, 0.5f);
    _normalThresholdSlider->Slider()->SetValue(pp->GetNormalThreshold());
    _normalThresholdSlider->Slider()->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);
    _normalThresholdSlider->Dock(Gwen::Pos::Top);

    // Luminance settings
    _lumDetectionCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _lumDetectionCheckBox->Label()->SetText("Luminance detection enabled");
    _lumDetectionCheckBox->Checkbox()->SetChecked(pp->GetLuminanceDetectionEnabled());
    _lumDetectionCheckBox->Dock(Gwen::Pos::Top);
    _lumDetectionCheckBox->Checkbox()->onCheckChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);

    _lumThresholdSlider = new SliderWithLabel(this);
    _lumThresholdSlider->Slider()->SetClampToNotches(false);
    _lumThresholdSlider->Slider()->SetRange(EPSILON, 0.5f);
    _lumThresholdSlider->Slider()->SetValue(pp->GetLuminanceThreshold());
    _lumThresholdSlider->Slider()->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);
    _lumThresholdSlider->Dock(Gwen::Pos::Top);

    // Search steps setting
    _maxSearchStepsSlider = new SliderWithLabel(this);
    _maxSearchStepsSlider->Slider()->SetClampToNotches(true);
    _maxSearchStepsSlider->Slider()->SetRange(pp->GetMinSearchSteps(), pp->GetMaxSearchSteps());
    _maxSearchStepsSlider->Slider()->SetNotchCount(pp->GetMaxSearchSteps() - pp->GetMinSearchSteps() + 1);
    _maxSearchStepsSlider->Slider()->SetValue(pp->GetSearchSteps());
    _maxSearchStepsSlider->Slider()->onValueChanged.Add(this, &MLAAConfigurationPane::OnValueChanged);
    _maxSearchStepsSlider->Dock(Gwen::Pos::Top);
}

void MLAAConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
    MLAAPostProcess* pp = GetConfiguredObject();

    if (control == _depthDetectionCheckBox->Checkbox())
    {
        pp->SetDepthDetectionEnabled(_depthDetectionCheckBox->Checkbox()->IsChecked());
    }
    else if (control == _depthThresholdSlider->Slider())
    {
        pp->SetDepthThreshold(_depthThresholdSlider->Slider()->GetValue());
    }
    else if (control == _normalDetectionCheckBox->Checkbox())
    {
        pp->SetNormalDetectionEnabled(_normalDetectionCheckBox->Checkbox()->IsChecked());
    }
    else if (control == _normalThresholdSlider->Slider())
    {
        pp->SetNormalThreshold(_normalThresholdSlider->Slider()->GetValue());
    }
    else if (control == _lumDetectionCheckBox->Checkbox())
    {
        pp->SetLuminanceDetectionEnabled(_lumDetectionCheckBox->Checkbox()->IsChecked());
    }
    else if (control == _lumThresholdSlider->Slider())
    {
        pp->SetLuminanceThreshold(_lumThresholdSlider->Slider()->GetValue());
    }
    else if (control == _maxSearchStepsSlider->Slider())
    {
        pp->SetSearchSteps(floor(_maxSearchStepsSlider->Slider()->GetValue() + 0.5f));
    }
}

void MLAAConfigurationPane::OnFrameMove(double totalTime, float dt)
{
    MLAAPostProcess* pp = GetConfiguredObject();

    _depthDetectionCheckBox->Checkbox()->SetChecked(pp->GetDepthDetectionEnabled());

    _depthThresholdSlider->Slider()->SetValue(pp->GetDepthThreshold());
    _depthThresholdSlider->Label()->SetText("Depth threshold: " + Gwen::Utility::ToString(pp->GetDepthThreshold()));

    _normalDetectionCheckBox->Checkbox()->SetChecked(pp->GetNormalDetectionEnabled());

    _normalThresholdSlider->Slider()->SetValue(pp->GetNormalThreshold());
    _normalThresholdSlider->Label()->SetText("Normal threshold: " + Gwen::Utility::ToString(pp->GetNormalThreshold()));

    _lumDetectionCheckBox->Checkbox()->SetChecked(pp->GetLuminanceDetectionEnabled());

    _lumThresholdSlider->Slider()->SetValue(pp->GetLuminanceThreshold());
    _lumThresholdSlider->Label()->SetText("Luminance threshold: " + Gwen::Utility::ToString(pp->GetLuminanceThreshold()));

    _maxSearchStepsSlider->Slider()->SetValue(pp->GetSearchSteps());
    _maxSearchStepsSlider->Label()->SetText("Search steps: " + Gwen::Utility::ToString(pp->GetSearchSteps()));
}