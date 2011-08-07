#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "TestingCamera.h"
#include "Gwen/Controls/HorizontalSlider.h"

class CameraConfigurationPane : public ConfigurationPane<TestingCamera>
{
private:
	Gwen::Controls::Label* _cameraPositionLabel;

	Gwen::Controls::Label* _cameraRotationLabel;

	Gwen::Controls::Label* _cameraFovLabel;
	Gwen::Controls::HorizontalSlider* _cameraFovSlider;

	Gwen::Controls::Label* _cameraNearClipLabel;
	Gwen::Controls::HorizontalSlider* _cameraNearClipSlider;

	Gwen::Controls::Label* _cameraFarClipLabel;
	Gwen::Controls::HorizontalSlider* _cameraFarClipSlider;

	Gwen::Controls::Label* _cameraMoveSpeedLabel;
	Gwen::Controls::HorizontalSlider* _cameraMoveSpeedSlider;

	Gwen::Controls::Label* _cameraRotSpeedLabel;
	Gwen::Controls::HorizontalSlider* _cameraRotSpeedSlider;

public:
	CameraConfigurationPane(Gwen::Controls::Base* parent, TestingCamera* cam);
	~CameraConfigurationPane();

	void OnFrameMove(double totalTime, float dt);
};