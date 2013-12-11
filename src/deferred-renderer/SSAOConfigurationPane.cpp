#include "PCH.h"
#include "SSAOConfigurationPane.h"

SSAOConfigurationPane::SSAOConfigurationPane(Gwen::Controls::Base* parent, SSAOPostProcess* pp)
    : ConfigurationPane(parent, L"SSAO", pp)
{
    _halfResCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _halfResCheckBox->Label()->SetText("Half resolution");
    _halfResCheckBox->Checkbox()->SetChecked(pp->GetHalfResolution());
    _halfResCheckBox->Dock(Gwen::Pos::Top);
    _halfResCheckBox->Checkbox()->onCheckChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);

    _sampleCountSlider = new SliderWithLabel(this);
    _sampleCountSlider->Slider()->SetClampToNotches(true);
    _sampleCountSlider->Slider()->SetRange(0.0f, pp->GetNumSampleCountIndices() - 1);
    _sampleCountSlider->Slider()->SetNotchCount(pp->GetNumSampleCountIndices());
    _sampleCountSlider->Slider()->SetValue(pp->GetSampleCountIndex());
    _sampleCountSlider->Slider()->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);
    _sampleCountSlider->Dock(Gwen::Pos::Top);

    _sampleRadiusSlider = new SliderWithLabel(this);
    _sampleRadiusSlider->Slider()->SetRange(0.0f, 50.0f);
    _sampleRadiusSlider->Slider()->SetValue(pp->GetSampleRadius());
    _sampleRadiusSlider->Slider()->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);
    _sampleRadiusSlider->Dock(Gwen::Pos::Top);

    _depthThresholdSlider = new SliderWithLabel(this);
    _depthThresholdSlider->Slider()->SetRange(EPSILON, 10.0f);
    _depthThresholdSlider->Slider()->SetValue(pp->GetDepthThreshold());
    _depthThresholdSlider->Slider()->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);
    _depthThresholdSlider->Dock(Gwen::Pos::Top);

    _blurSigmaSlider = new SliderWithLabel(this);
    _blurSigmaSlider->Slider()->SetRange(EPSILON, 4.0f);
    _blurSigmaSlider->Slider()->SetValue(pp->GetBlurSigma());
    _blurSigmaSlider->Slider()->onValueChanged.Add(this, &SSAOConfigurationPane::OnValueChanged);
    _blurSigmaSlider->Dock(Gwen::Pos::Top);
}

void SSAOConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
    SSAOPostProcess* pp = GetConfiguredObject();

    if (control == _halfResCheckBox->Checkbox())
    {
        pp->SetHalfResolution(_halfResCheckBox->Checkbox()->IsChecked());
    }
    else if (control == _sampleCountSlider->Slider())
    {
        pp->SetSampleCountIndex(floor(_sampleCountSlider->Slider()->GetValue() + 0.5f));
    }
    else if (control == _sampleRadiusSlider->Slider())
    {
        pp->SetSampleRadius(_sampleRadiusSlider->Slider()->GetValue());
    }
    else if (control == _depthThresholdSlider->Slider())
    {
        pp->SetDepthTreshold(_depthThresholdSlider->Slider()->GetValue());
    }
    else if (control == _blurSigmaSlider->Slider())
    {
        pp->SetBlurSigma(_blurSigmaSlider->Slider()->GetValue());
    }
}

void SSAOConfigurationPane::OnFrameMove(double totalTime, float dt)
{
    SSAOPostProcess* pp = GetConfiguredObject();

    _halfResCheckBox->Checkbox()->SetChecked(pp->GetHalfResolution());

    _sampleCountSlider->Slider()->SetValue(pp->GetSampleCountIndex());
    _sampleCountSlider->Label()->SetText("Sample count: " + Gwen::Utility::ToString(pp->GetSampleCount()));

    _sampleRadiusSlider->Slider()->SetValue(pp->GetSampleRadius());
    _sampleRadiusSlider->Label()->SetText("Sample radius: " + Gwen::Utility::ToString(pp->GetSampleRadius()));

    _depthThresholdSlider->Slider()->SetValue(pp->GetDepthThreshold());
    _depthThresholdSlider->Label()->SetText("Depth Threshold: " + Gwen::Utility::ToString(pp->GetDepthThreshold()));

    _blurSigmaSlider->Slider()->SetValue(pp->GetBlurSigma());
    _blurSigmaSlider->Label()->SetText("Blur sigma: " + Gwen::Utility::ToString(pp->GetBlurSigma()));
}