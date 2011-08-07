#pragma once

#include "PCH.h"

class ContentTypeBase
{
public:
	virtual HRESULT LoadContent(ID3D11Device* device) = 0;
	virtual void UnloadContent() = 0;
};