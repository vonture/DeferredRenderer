#include "DeviceManagerConfigurationPane.h"

DeviceManagerConfigurationPane::DeviceManagerConfigurationPane(Gwen::Controls::Base* parent, 
	DeviceManager* manager)
	: ConfigurationPane(parent, manager)
{
	SetName("Graphics Device");
}

void DeviceManagerConfigurationPane::OnFrameMove(double totalTime, float dt)
{
}