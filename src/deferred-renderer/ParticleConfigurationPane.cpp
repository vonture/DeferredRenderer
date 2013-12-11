#include "PCH.h"
#include "ParticleConfigurationPane.h"

ParticleConfigurationPane::ParticleConfigurationPane(Gwen::Controls::Base* parent)
    : ConfigurationPaneBase(parent, L"Particles")
{
    const int labelHeight = 20;

    _grav = XMFLOAT3(0.0f, -1.0f, 0.0f);
    _wind = XMFLOAT3(0.0f, 0.0f, 0.0f);

    _windDirLabel = new Gwen::Controls::Label(this);
    _windDirLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
    _windDirLabel->SetHeight(labelHeight);
    _windDirLabel->Dock(Gwen::Pos::Top);

    _windDirSelect = new DirectionSelector(this);
    _windDirSelect->SetDirection(XMFLOAT2(0.0f, 0.0f));
    _windDirSelect->onValueChanged.Add(this, &ParticleConfigurationPane::OnValueChanged);
    _windDirSelect->Dock(Gwen::Pos::Top);

    _windStrengthSlider = new SliderWithLabel(this);
    _windStrengthSlider->Slider()->SetRange(0.0f, 10.0f);
    _windStrengthSlider->Slider()->SetValue(0.0f);
    _windStrengthSlider->Slider()->onValueChanged.Add(this, &ParticleConfigurationPane::OnValueChanged);
    _windStrengthSlider->Dock(Gwen::Pos::Top);

    _gravityDirLabel = new Gwen::Controls::Label(this);
    _gravityDirLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
    _gravityDirLabel->SetHeight(labelHeight);
    _gravityDirLabel->Dock(Gwen::Pos::Top);

    _gravityDirSelect = new DirectionSelector(this);
    _gravityDirSelect->SetDirection(XMFLOAT2(0.0f, 0.0f));
    _gravityDirSelect->onValueChanged.Add(this, &ParticleConfigurationPane::OnValueChanged);
    _gravityDirSelect->Dock(Gwen::Pos::Top);

    _gravityStrengthSlider = new SliderWithLabel(this);
    _gravityStrengthSlider->Slider()->SetRange(-10.0f, 10.0f);
    _gravityStrengthSlider->Slider()->SetValue(-1.0f);
    _gravityStrengthSlider->Slider()->onValueChanged.Add(this, &ParticleConfigurationPane::OnValueChanged);
    _gravityStrengthSlider->Dock(Gwen::Pos::Top);
}

const XMFLOAT3& ParticleConfigurationPane::GetGravityVector() const
{
    return _grav;
}

const XMFLOAT3& ParticleConfigurationPane::GetWindVector() const
{
    return _wind;
}

void ParticleConfigurationPane::AddParticleInstance(ParticleSystemInstance* inst)
{
    _particles.push_back(inst);
}

void ParticleConfigurationPane::SelectParticleInstance(ParticleSystemInstance* inst)
{
}

ParticleSystemInstance* ParticleConfigurationPane::GetParticleInstance(UINT idx)
{
    return _particles[idx];
}

UINT ParticleConfigurationPane::GetParticleInstanceCount() const
{
    return _particles.size();
}

void ParticleConfigurationPane::OnValueChanged(Gwen::Controls::Base *control)
{
    if (control == _gravityDirSelect || control == _gravityStrengthSlider->Slider())
    {
        XMFLOAT2 dir = _gravityDirSelect->GetDirection();
        float y = sqrtf(1.0f - (dir.x * dir.x + dir.y * dir.y));

        float mag = _gravityStrengthSlider->Slider()->GetValue();

        _grav = XMFLOAT3(dir.x * fabs(mag), y * mag, dir.y * fabs(mag));
    }
    else if (control == _windDirSelect || control == _windStrengthSlider->Slider())
    {
        XMFLOAT2 dir = _windDirSelect->GetDirection();

        float mag = _windStrengthSlider->Slider()->GetValue();

        _wind = XMFLOAT3(dir.x * mag, 0.0f, dir.y * mag);
    }
}

void ParticleConfigurationPane::OnFrameMove( double totalTime, float dt )
{
    _gravityDirLabel->SetText("Gravity direction: (" + Gwen::Utility::ToString(_grav.x) +
        ", " + Gwen::Utility::ToString(_grav.y) + ", " + Gwen::Utility::ToString(_grav.z) + ")");

    float gravStr = _gravityStrengthSlider->Slider()->GetValue();
    _gravityStrengthSlider->Label()->SetText("Gravity strength: " + Gwen::Utility::ToString(gravStr));

    _windDirLabel->SetText("Wind direction: (" + Gwen::Utility::ToString(_wind.x) +
        ", " + Gwen::Utility::ToString(_wind.y) + ", " + Gwen::Utility::ToString(_wind.z) + ")");

    float windStr = _windStrengthSlider->Slider()->GetValue();
    _windStrengthSlider->Label()->SetText("Wind strength: " + Gwen::Utility::ToString(windStr));
}