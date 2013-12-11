#include "PCH.h"
#include "BoundingObjectConfigurationPane.h"

BoundingObjectConfigurationPane::BoundingObjectConfigurationPane(Gwen::Controls::Base* parent,
                                                                 BoundingObjectPostProcess* pp)
                                                                 : ConfigurationPane(parent, L"Bounding Objects", pp)
{
    const int labelHeight = 20;

    _modelEnableCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _modelEnableCheckBox->Label()->SetText("Models enabled");
    _modelEnableCheckBox->Checkbox()->SetChecked(false);
    _modelEnableCheckBox->Dock(Gwen::Pos::Top);

    _particleEnableCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _particleEnableCheckBox->Label()->SetText("Particles enabled");
    _particleEnableCheckBox->Checkbox()->SetChecked(false);
    _particleEnableCheckBox->Dock(Gwen::Pos::Top);

    _lightEnableCheckBox = new Gwen::Controls::CheckBoxWithLabel(this);
    _lightEnableCheckBox->Label()->SetText("Lights enabled");
    _lightEnableCheckBox->Checkbox()->SetChecked(false);
    _lightEnableCheckBox->Dock(Gwen::Pos::Top);

    _boColorLabel = new Gwen::Controls::Label(this);
    _boColorLabel->SetText("Bounding object color:");
    _boColorLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
    _boColorLabel->SetHeight(labelHeight);
    _boColorLabel->Dock(Gwen::Pos::Top);

    _boColorPicker = new Gwen::Controls::ColorPicker(this);
    _boColorPicker->SetColor(Vector4ToGwenColor(pp->GetColor()));
    _boColorPicker->Dock(Gwen::Pos::Top);
    _boColorPicker->onColorChanged.Add(this, &BoundingObjectConfigurationPane::OnValueChanged);
}

void BoundingObjectConfigurationPane::OnValueChanged( Gwen::Controls::Base *control )
{
    BoundingObjectPostProcess* pp = GetConfiguredObject();

    if (control == _boColorPicker)
    {
        pp->SetColor(GwenColorToVector4(_boColorPicker->GetColor()));
    }
}

void BoundingObjectConfigurationPane::OnFrameMove(double totalTime, float dt)
{
    BoundingObjectPostProcess* pp = GetConfiguredObject();

    _boColorPicker->SetColor(Vector4ToGwenColor(pp->GetColor()));
}