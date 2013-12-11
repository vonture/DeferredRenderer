#pragma once

#include "PCH.h"
#include "ConfigurationPane.h"
#include "DeviceManager.h"

class DeviceManagerConfigurationPane : public ConfigurationPane<DeviceManager>
{
public:
    DeviceManagerConfigurationPane(Gwen::Controls::Base* parent, DeviceManager* manager);

    void OnFrameMove(double totalTime, float dt);
};