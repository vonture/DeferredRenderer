#pragma once

#include "PCH.h"
#include "IUpdateable.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Utility.h"

class ConfigurationPaneBase : public Gwen::Controls::Base, public IUpdateable
{
public:
    ConfigurationPaneBase(Gwen::Controls::Base* parent, const WCHAR* name)
        : Gwen::Controls::Base(parent)
    {
        SetName(Gwen::Utility::UnicodeToString(name));
    }

    virtual void OnFrameMove(double totalTime, float dt) { };

protected:
    Gwen::Color Vector3ToGwenColor(const XMFLOAT3& vector)
    {
        return Gwen::Color(vector.x * 255, vector.y * 255, vector.z * 255);
    }
    Gwen::Color Vector4ToGwenColor(const XMFLOAT4& vector)
    {
        return Gwen::Color(vector.x * 255, vector.y * 255, vector.z * 255, vector.w * 255);
    }

    XMFLOAT3 GwenColorToVector3(const Gwen::Color& color)
    {
        return XMFLOAT3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
    }
    XMFLOAT4 GwenColorToVector4(const Gwen::Color& color)
    {
        return XMFLOAT4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};