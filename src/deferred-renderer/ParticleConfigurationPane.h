#pragma once

#include "PCH.h"
#include "ConfigurationPaneBase.h"
#include "ParticleSystemInstance.h"
#include "Gwen/Controls/TreeControl.h"
#include "Gwen/Controls/Properties.h"
#include "Gwen/Controls/Label.h"
#include "SliderWithLabel.h"
#include "DirectionSelector.h"

class ParticleConfigurationPane : public ConfigurationPaneBase
{
private:
	std::vector<ParticleSystemInstance*> _particles;

	XMFLOAT3 _grav;
	XMFLOAT3 _wind;

	Gwen::Controls::Label* _gravityDirLabel;
	DirectionSelector* _gravityDirSelect;
	SliderWithLabel* _gravityStrengthSlider;

	Gwen::Controls::Label* _windDirLabel;
	DirectionSelector* _windDirSelect;
	SliderWithLabel* _windStrengthSlider;

	void OnValueChanged(Gwen::Controls::Base *control);

public:
	ParticleConfigurationPane(Gwen::Controls::Base* parent);

	const XMFLOAT3& GetGravityVector() const;
	const XMFLOAT3& GetWindVector() const;

	void AddParticleInstance(ParticleSystemInstance* inst);

	void SelectParticleInstance(ParticleSystemInstance* inst);

	ParticleSystemInstance* GetParticleInstance(UINT idx);
	UINT GetParticleInstanceCount() const;

	void OnFrameMove(double totalTime, float dt);
};