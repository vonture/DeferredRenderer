#include "CameraConfigurationPane.h"

CameraConfigurationPane::CameraConfigurationPane(Gwen::Controls::Base* parent, TestingCamera* cam)
	: ConfigurationPane(parent, L"Camera", cam)
{	
	const int childWidth = 240;
	const int labelHight = 20;
	const int sliderHeight = 14;
	const int spacing = 3;

	// Position
	_cameraPositionLabel = new Gwen::Controls::Label(this);
	_cameraPositionLabel->SetBounds(0, 0, childWidth, labelHight);
	_cameraPositionLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraPositionLabel->SetText("Camera position:");

	// Rotation
	_cameraRotationLabel = new Gwen::Controls::Label(this);
	_cameraRotationLabel->SetBounds(0, _cameraPositionLabel->Bottom() + spacing, childWidth, labelHight);
	_cameraRotationLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraRotationLabel->SetText("Camera rotation:");

	// fov
	_cameraFovLabel = new Gwen::Controls::Label(this);
	_cameraFovLabel->SetBounds(0, _cameraRotationLabel->Bottom() + spacing, childWidth, labelHight);
	_cameraFovLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraFovLabel->SetText("");

	_cameraFovSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraFovSlider->SetClampToNotches(false);
	_cameraFovSlider->SetRange(EPSILON, Pi - EPSILON);
	_cameraFovSlider->SetValue(cam->GetFieldOfView());
	_cameraFovSlider->SetBounds(0, _cameraFovLabel->Bottom(), childWidth, sliderHeight);

	// near
	_cameraNearClipLabel = new Gwen::Controls::Label(this);
	_cameraNearClipLabel->SetBounds(0, _cameraFovSlider->Bottom() + spacing, childWidth, labelHight);
	_cameraNearClipLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraNearClipLabel->SetText("");

	_cameraNearClipSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraNearClipSlider->SetClampToNotches(false);
	_cameraNearClipSlider->SetRange(EPSILON, 100.0f);
	_cameraNearClipSlider->SetValue(cam->GetNearClip());
	_cameraNearClipSlider->SetBounds(0, _cameraNearClipLabel->Bottom(), childWidth, sliderHeight);

	// far
	_cameraFarClipLabel = new Gwen::Controls::Label(this);
	_cameraFarClipLabel->SetBounds(0, _cameraNearClipSlider->Bottom() + spacing, childWidth, labelHight);
	_cameraFarClipLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraFarClipLabel->SetText("");

	_cameraFarClipSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraFarClipSlider->SetClampToNotches(false);
	_cameraFarClipSlider->SetRange(EPSILON, 100.0f);
	_cameraFarClipSlider->SetValue(cam->GetFarClip());
	_cameraFarClipSlider->SetBounds(0, _cameraFarClipLabel->Bottom(), childWidth, sliderHeight);

	// move speed
	_cameraMoveSpeedLabel = new Gwen::Controls::Label(this);
	_cameraMoveSpeedLabel->SetBounds(0, _cameraFarClipSlider->Bottom() + spacing, childWidth, labelHight);
	_cameraMoveSpeedLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraMoveSpeedLabel->SetText("");

	_cameraMoveSpeedSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraMoveSpeedSlider->SetClampToNotches(false);
	_cameraMoveSpeedSlider->SetRange(0.0f, 100.0f);
	_cameraMoveSpeedSlider->SetValue(cam->GetMovementSpeed());
	_cameraMoveSpeedSlider->SetBounds(0, _cameraMoveSpeedLabel->Bottom(), childWidth, sliderHeight);

	// rot speed
	_cameraRotSpeedLabel = new Gwen::Controls::Label(this);
	_cameraRotSpeedLabel->SetBounds(0, _cameraMoveSpeedSlider->Bottom() + spacing, childWidth, labelHight);
	_cameraRotSpeedLabel->SetAlignment(Gwen::Pos::Bottom | Gwen::Pos::Left);
	_cameraRotSpeedLabel->SetText("");

	_cameraRotSpeedSlider = new Gwen::Controls::HorizontalSlider(this);
	_cameraRotSpeedSlider->SetClampToNotches(false);
	_cameraRotSpeedSlider->SetRange(0.0f, 0.01f);
	_cameraRotSpeedSlider->SetValue(cam->GetRotationSpeed());
	_cameraRotSpeedSlider->SetBounds(0, _cameraRotSpeedLabel->Bottom(), childWidth, sliderHeight);
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