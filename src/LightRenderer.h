#pragma once

#include "PCH.h"
#include "LightRendererBase.h"

template <class T>
class LightRenderer : public LightRendererBase
{
private:
	static const UINT MAX_LIGHTS = 1024;

	UINT _shadowedCount;
	T* _shadowed[MAX_LIGHTS];
	
	UINT _unshadowedCount;
	T* _unshadowed[MAX_LIGHTS];

public:
	LightRenderer()
		: _shadowedCount(0), _unshadowedCount(0)
	{
	}
	
	void Add(T* light, bool shadowed) 
	{
		if (shadowed && _shadowedCount < max(GetMaxShadowedLights(), MAX_LIGHTS))
		{
			_shadowed[_shadowedCount] = light;
			_shadowedCount++;
		}
		else if (_unshadowedCount < MAX_LIGHTS)
		{
			_unshadowed[_unshadowedCount] = light;
			_unshadowedCount++;
		}
	}

	UINT GetCount() { return _shadowedCount + _unshadowedCount; }
	UINT GetCount(bool shadowed) { return shadowed ? _shadowedCount : _unshadowedCount; }

	T* GetLight(UINT idx)
    {
		return (idx >= _shadowedCount) ? _unshadowed[idx] :  _shadowed[idx - _shadowedCount];
    }

	T* GetLight(UINT idx, bool shadowed)
    {
        return shadowed ? _shadowed[idx] : _unshadowed[idx];
    }
	
	void Clear()
	{
		_shadowedCount = 0;
		_unshadowedCount = 0;
	}
};
