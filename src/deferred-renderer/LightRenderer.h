#pragma once

#include "PCH.h"
#include "LightRendererBase.h"

template <class T>
class LightRenderer : public LightRendererBase
{
private:
	std::vector<T*> _shadowed;
	std::vector<T*> _unshadowed;

public:
	LightRenderer() { }
	virtual ~LightRenderer() { }
	
	void Add(T* light, bool shadowed) 
	{
		if (shadowed && _shadowed.size() < GetMaxShadowedLights())
		{
			_shadowed.push_back(light);
		}
		else
		{
			_unshadowed.push_back(light);
		}
	}

	UINT GetCount() { return _shadowed.size() + _unshadowed.size(); }
	UINT GetCount(bool shadowed) { return shadowed ? _shadowed.size() : _unshadowed.size(); }

	T* GetLight(UINT idx)
    {
		return (idx >= _shadowed.size()) ? _unshadowed[idx] :  _shadowed[idx - _shadowed.size()];
    }

	T* GetLight(UINT idx, bool shadowed)
    {
        return shadowed ? _shadowed[idx] : _unshadowed[idx];
    }
	
	void Clear()
	{
		_shadowed.clear();
		_unshadowed.clear();
	}
};
