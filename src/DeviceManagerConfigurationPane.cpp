#include "DeviceManagerConfigurationPane.h"

DeviceManagerConfigurationPane::DeviceManagerConfigurationPane(Gwen::Controls::Base* parent, 
	DeviceManager* manager)
	: ConfigurationPane(parent, L"Graphics Device", manager)
{
}

void DeviceManagerConfigurationPane::OnFrameMove(double totalTime, float dt)
{
}