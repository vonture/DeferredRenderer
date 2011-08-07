#pragma once

#include "PCH.h"
#include "Gwen/Controls/Base.h"
#include "FloatUpDown.h"

class Float2UpDown : public Gwen::Controls::Base
{
private:
	Gwen::Controls::Label* _xLabel;
	FloatUpDown* _xUpDown;

	Gwen::Controls::Label* _yLabel;
	FloatUpDown* _yUpDown;
	
	virtual void OnUpDownChanged(Gwen::Controls::Base* control );

public:	
	Float2UpDown(Gwen::Controls::Base *parent);
	~Float2UpDown();

	XMFLOAT2 GetMin();
	void SetMin(const XMFLOAT2& min);

	XMFLOAT2 GetMax();
	void SetMax(const XMFLOAT2& max);

	XMFLOAT2 GetValue();
	void SetValue(const XMFLOAT2& val);

	Gwen::Event::Caller onChanged;
};