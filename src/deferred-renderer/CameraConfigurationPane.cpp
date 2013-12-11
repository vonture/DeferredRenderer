#include "PCH.h"
#include "CameraConfigurationPane.h"

CameraConfigurationPane::CameraConfigurationPane(Gwen::Controls::Base* parent, TestingCamera* cam)
    : ConfigurationPane(parent, L"Camera", cam)
{
    const int labelHeight = 20;

    // Position
    _cameraPositionLabel = new Gwen::Controls::Label(this);
    _cameraPositionLabel->SetHeight(labelHeight);
    _cameraPositionLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
    _cameraPositionLabel->Dock(Gwen::Pos::Top);

    // Rotation
    _cameraRotationLabel = new Gwen::Controls::Label(this);
    _cameraRotationLabel->SetHeight(labelHeight);
    _cameraRotationLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
    _cameraRotationLabel->Dock(Gwen::Pos::Top);

    // fov
    _cameraFovSlider = new SliderWithLabel(this);
    _cameraFovSlider->Slider()->SetRange(EPSILON, Pi - EPSILON);
    _cameraFovSlider->Slider()->SetValue(cam->GetFieldOfView());
    _cameraFovSlider->Slider()->onValueChanged.Add(this, &CameraConfigurationPane::onValueChanged);
    _cameraFovSlider->Dock(Gwen::Pos::Top);

    // near
    _cameraNearClipSlider = new SliderWithLabel(this);
    _cameraNearClipSlider->Slider()->SetRange(EPSILON, 100.0f);
    _cameraNearClipSlider->Slider()->SetValue(cam->GetNearClip());
    _cameraNearClipSlider->Slider()->onValueChanged.Add(this, &CameraConfigurationPane::onValueChanged);
    _cameraNearClipSlider->Dock(Gwen::Pos::Top);

    // far
    _cameraFarClipSlider = new SliderWithLabel(this);
    _cameraFarClipSlider->Slider()->SetRange(EPSILON, 500.0f);
    _cameraFarClipSlider->Slider()->SetValue(cam->GetFarClip());
    _cameraFarClipSlider->Slider()->onValueChanged.Add(this, &CameraConfigurationPane::onValueChanged);
    _cameraFarClipSlider->Dock(Gwen::Pos::Top);

    // move speed
    _cameraMoveSpeedSlider = new SliderWithLabel(this);
    _cameraMoveSpeedSlider->Slider()->SetRange(0.0f, 100.0f);
    _cameraMoveSpeedSlider->Slider()->SetValue(cam->GetMovementSpeed());
    _cameraMoveSpeedSlider->Slider()->onValueChanged.Add(this, &CameraConfigurationPane::onValueChanged);
    _cameraMoveSpeedSlider->Dock(Gwen::Pos::Top);

    // rot speed
    _cameraRotSpeedSlider = new SliderWithLabel(this);
    _cameraRotSpeedSlider->Slider()->SetRange(0.0f, 0.01f);
    _cameraRotSpeedSlider->Slider()->SetValue(cam->GetRotationSpeed());
    _cameraRotSpeedSlider->Slider()->onValueChanged.Add(this, &CameraConfigurationPane::onValueChanged);
    _cameraRotSpeedSlider->Dock(Gwen::Pos::Top);
}

void CameraConfigurationPane::onValueChanged(Gwen::Controls::Base* control)
{
    TestingCamera* cam = GetConfiguredObject();

    if (control == _cameraFovSlider->Slider())
    {
        cam->SetFieldOfView(_cameraFovSlider->Slider()->GetValue());
    }
    else if (control == _cameraNearClipSlider->Slider() || control == _cameraFarClipSlider->Slider())
    {
        cam->SetClips(_cameraNearClipSlider->Slider()->GetValue(), _cameraFarClipSlider->Slider()->GetValue());
    }
    else if (control == _cameraMoveSpeedSlider->Slider())
    {
        cam->SetMovementSpeed(_cameraMoveSpeedSlider->Slider()->GetValue());
    }
    else if (control == _cameraRotSpeedSlider->Slider())
    {
        cam->SetRotationSpeed(_cameraRotSpeedSlider->Slider()->GetValue());
    }
}

void CameraConfigurationPane::OnFrameMove(double totalTime, float dt)
{
    TestingCamera* cam = GetConfiguredObject();

    // Update camera pos
    XMFLOAT3 camPos = cam->GetPosition();
    _cameraPositionLabel->SetText("Position: (" + Gwen::Utility::ToString(camPos.x) + ", " +
        Gwen::Utility::ToString(camPos.y) + ", " + Gwen::Utility::ToString(camPos.z) + ")");

    // update camera rot
    XMFLOAT2 camRot = cam->GetRotation();
    _cameraRotationLabel->SetText("Rotation: (" + Gwen::Utility::ToString(camRot.x) + ", " +
        Gwen::Utility::ToString(camRot.y) + ")");

    // fov
    _cameraFovSlider->Slider()->SetValue(cam->GetFieldOfView());
    _cameraFovSlider->Label()->SetText("Field of view: " + Gwen::Utility::ToString(cam->GetFieldOfView()));

    // near
    _cameraNearClipSlider->Slider()->SetValue(cam->GetNearClip());
    _cameraNearClipSlider->Label()->SetText("Near clip: " + Gwen::Utility::ToString(cam->GetNearClip()));

    // far
    _cameraFarClipSlider->Slider()->SetValue(cam->GetFarClip());
    _cameraFarClipSlider->Label()->SetText("Far clip: " + Gwen::Utility::ToString(cam->GetFarClip()));

    // move speed
    _cameraMoveSpeedSlider->Slider()->SetValue(cam->GetMovementSpeed());
    _cameraMoveSpeedSlider->Label()->SetText("Movement speed: " + Gwen::Utility::ToString(cam->GetMovementSpeed()));

    // rot speed
    _cameraRotSpeedSlider->Slider()->SetValue(cam->GetRotationSpeed());
    _cameraRotSpeedSlider->Label()->SetText("Rotation speed: " + Gwen::Utility::ToString(cam->GetRotationSpeed()));
}