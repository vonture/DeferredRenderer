#include "PCH.h"
#include "FXAAConfigurationPane.h"

FXAAConfigurationPane::FXAAConfigurationPane(Gwen::Controls::Base* parent, FXAAPostProcess* pp)
    : ConfigurationPane(parent, L"FXAA", pp)
{
    _qualityPresetSlider = new SliderWithLabel(this);
    _qualityPresetSlider->Slider()->SetClampToNotches(true);
    _qualityPresetSlider->Slider()->SetRange(0.0f, pp->GetQualityPresetIndexCount() - 1);
    _qualityPresetSlider->Slider()->SetNotchCount(pp->GetQualityPresetIndexCount());
    _qualityPresetSlider->Slider()->SetValue(pp->GetQualityPresetIndex());
    _qualityPresetSlider->Slider()->onValueChanged.Add(this, &FXAAConfigurationPane::OnValueChanged);
    _qualityPresetSlider->Dock(Gwen::Pos::Top);

    _subpixelSlider = new SliderWithLabel(this);
    _subpixelSlider->Slider()->SetRange(0.0f, 1.0f);
    _subpixelSlider->Slider()->SetValue(pp->GetSubpixelAliasingRemoval());
    _subpixelSlider->Slider()->onValueChanged.Add(this, &FXAAConfigurationPane::OnValueChanged);
    _subpixelSlider->Dock(Gwen::Pos::Top);

    _edgeThresholdSlider = new SliderWithLabel(this);
    _edgeThresholdSlider->Slider()->SetRange(0.0f, 0.5f);
    _edgeThresholdSlider->Slider()->SetValue(pp->GetEdgeThreshold());
    _edgeThresholdSlider->Slider()->onValueChanged.Add(this, &FXAAConfigurationPane::OnValueChanged);
    _edgeThresholdSlider->Dock(Gwen::Pos::Top);

    _edgeThresholdMinSlider = new SliderWithLabel(this);
    _edgeThresholdMinSlider->Slider()->SetRange(0.0f, 0.0833f);
    _edgeThresholdMinSlider->Slider()->SetValue(pp->GetMinimumEdgeThreshold());
    _edgeThresholdMinSlider->Slider()->onValueChanged.Add(this, &FXAAConfigurationPane::OnValueChanged);
    _edgeThresholdMinSlider->Dock(Gwen::Pos::Top);
}

void FXAAConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
    FXAAPostProcess* pp = GetConfiguredObject();

    if (control == _qualityPresetSlider->Slider())
    {
        pp->SetQualityPresetIndex(floor(_qualityPresetSlider->Slider()->GetValue() + 0.5f));
    }
    else if (control == _subpixelSlider->Slider())
    {
        pp->SetSubpixelAliasingRemoval(_subpixelSlider->Slider()->GetValue());
    }
    else if (control == _edgeThresholdSlider->Slider())
    {
        pp->SetEdgeThreshold(_edgeThresholdSlider->Slider()->GetValue());
    }
    else if (control == _edgeThresholdMinSlider->Slider())
    {
        pp->SetMinimumEdgeThreshold(_edgeThresholdMinSlider->Slider()->GetValue());
    }
}

void FXAAConfigurationPane::OnFrameMove(double totalTime, float dt)
{
    FXAAPostProcess* pp = GetConfiguredObject();

    _qualityPresetSlider->Slider()->SetValue(pp->GetQualityPresetIndex());
    _qualityPresetSlider->Label()->SetText("Quality preset: " + Gwen::Utility::ToString(pp->GetQualityPreset()));

    _subpixelSlider->Slider()->SetValue(pp->GetSubpixelAliasingRemoval());
    _subpixelSlider->Label()->SetText("Subpixel aliasing removal: " + Gwen::Utility::ToString(pp->GetSubpixelAliasingRemoval()));

    _edgeThresholdSlider->Slider()->SetValue(pp->GetEdgeThreshold());
    _edgeThresholdSlider->Label()->SetText("Edge threshold: " + Gwen::Utility::ToString(pp->GetEdgeThreshold()));

    _edgeThresholdMinSlider->Slider()->SetValue(pp->GetMinimumEdgeThreshold());
    _edgeThresholdMinSlider->Label()->SetText("Minimum edge threshold: " + Gwen::Utility::ToString(pp->GetMinimumEdgeThreshold()));
}