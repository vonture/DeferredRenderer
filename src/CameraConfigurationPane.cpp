#include "CameraConfigurationPane.h"

CameraConfigurationPane::CameraConfigurationPane(Gwen::Controls::Base* parent, TestingCamera* cam)
	: ConfigurationPane(parent, cam)
{
	SetName("Camera");
	
	// Position
	_cameraPositionLabel = new Gwen::Controls::Label(this);
	_cameraPositionLabel->SetHeight(16);
	_cameraPositionLabel->SetText("Camera position:");
	_cameraPositionLabel->Dock(Gwen::Pos::Top);

	// Rotation
	_cameraRotationLabel = new Gwen::Controls::Label(this);
	_cameraRotationLabel->SetHeight(16);
	_cameraRotationLabel->SetText("Camera rotation:");
	_cameraRotationLabel->Dock(Gwen::Pos::Top);

	// fov
	_cameraFovLabel = new Gwen::Controls::Label(this);
	_cameraFovLabel->SetHeight(16);
	_cameraFovLabel->SetText("");
	_cameraFovLabel->Dock(Gwen::Pos::Top);

	_cameraFovSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraFovSlider->SetClampToNotches(false);
	_cameraFovSlider->SetRange(0.0001f, Pi - 0.0001f);
	_cameraFovSlider->SetValue(cam->GetFieldOfView());
	_cameraFovSlider->SetHeight(16);
	_cameraFovSlider->Dock(Gwen::Pos::Top);	

	// near
	_cameraNearClipLabel = new Gwen::Controls::Label(this);
	_cameraNearClipLabel->SetHeight(16);
	_cameraNearClipLabel->SetText("");
	_cameraNearClipLabel->Dock(Gwen::Pos::Top);

	_cameraNearClipSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraNearClipSlider->SetClampToNotches(false);
	_cameraNearClipSlider->SetRange(0.0f, 100.0f);
	_cameraNearClipSlider->SetValue(cam->GetNearClip());
	_cameraNearClipSlider->SetHeight(16);
	_cameraNearClipSlider->Dock(Gwen::Pos::Top);	

	// far
	_cameraFarClipLabel = new Gwen::Controls::Label(this);
	_cameraFarClipLabel->SetHeight(16);
	_cameraFarClipLabel->SetText("");
	_cameraFarClipLabel->Dock(Gwen::Pos::Top);

	_cameraFarClipSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraFarClipSlider->SetClampToNotches(false);
	_cameraFarClipSlider->SetRange(0.0f, 100.0f);
	_cameraFarClipSlider->SetValue(cam->GetFarClip());
	_cameraFarClipSlider->SetHeight(16);
	_cameraFarClipSlider->Dock(Gwen::Pos::Top);	

	// move speed
	_cameraMoveSpeedLabel = new Gwen::Controls::Label(this);
	_cameraMoveSpeedLabel->SetHeight(16);
	_cameraMoveSpeedLabel->SetText("");
	_cameraMoveSpeedLabel->Dock(Gwen::Pos::Top);

	_cameraMoveSpeedSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraMoveSpeedSlider->SetClampToNotches(false);
	_cameraMoveSpeedSlider->SetRange(0.0f, 100.0f);
	_cameraMoveSpeedSlider->SetValue(cam->GetMovementSpeed());
	_cameraMoveSpeedSlider->SetHeight(16);
	_cameraMoveSpeedSlider->Dock(Gwen::Pos::Top);	

	// rot speed
	_cameraRotSpeedLabel = new Gwen::Controls::Label(this);
	_cameraRotSpeedLabel->SetHeight(16);
	_cameraRotSpeedLabel->SetText("");
	_cameraRotSpeedLabel->Dock(Gwen::Pos::Top);

	_cameraRotSpeedSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraRotSpeedSlider->SetClampToNotches(false);
	_cameraRotSpeedSlider->SetRange(0.0f, 0.01f);
	_cameraRotSpeedSlider->SetValue(cam->GetRotationSpeed());
	_cameraRotSpeedSlider->SetHeight(16);
	_cameraRotSpeedSlider->Dock(Gwen::Pos::Top);	
}

CameraConfigurationPane::~CameraConfigurationPane()
{
}

void CameraConfigurationPane::OnFrameMove(double totalTime, float dt)
{
	TestingCamera* cam = GetConfiguredObject();

	// Update camera pos
	XMFLOAT3 camPos = cam->GetPosition();
	_cameraPositionLabel->SetText("Position: (" + Gwen::Utility::ToString(camPos.x) + 
		", " + Gwen::Utility::ToString(camPos.y) + ", " + Gwen::Utility::ToString(camPos.z) + ")");

	// update camera rot
	XMFLOAT2 camRot = cam->GetRotation();
	_cameraRotationLabel->SetText("Rotation: (" + Gwen::Utility::ToString(camRot.x) + 
		", " + Gwen::Utility::ToString(camRot.y) + ")");

	// fov
	cam->SetFieldOfView(_cameraFovSlider->GetValue());
	_cameraFovSlider->SetValue(cam->GetFieldOfView());
	_cameraFovLabel->SetText("Field of view: " + Gwen::Utility::ToString(cam->GetFieldOfView()));

	// near
	cam->SetNearClip(_cameraNearClipSlider->GetValue());
	_cameraNearClipSlider->SetValue(cam->GetNearClip());
	_cameraNearClipLabel->SetText("Near clip: " + Gwen::Utility::ToString(cam->GetNearClip()));

	// far
	cam->SetFarClip(_cameraFarClipSlider->GetValue());
	_cameraFarClipSlider->SetValue(cam->GetFarClip());
	_cameraFarClipLabel->SetText("Far clip: " + Gwen::Utility::ToString(cam->GetFarClip()));

	// move speed
	cam->SetMovementSpeed(_cameraMoveSpeedSlider->GetValue());
	_cameraMoveSpeedSlider->SetValue(cam->GetMovementSpeed());
	_cameraMoveSpeedLabel->SetText("Movement speed: " + Gwen::Utility::ToString(cam->GetMovementSpeed()));

	// rot speed
	cam->SetRotationSpeed(_cameraRotSpeedSlider->GetValue());
	_cameraRotSpeedSlider->SetValue(cam->GetRotationSpeed());
	_cameraRotSpeedLabel->SetText("Rotation speed: " + Gwen::Utility::ToString(cam->GetRotationSpeed()));
}