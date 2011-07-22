#pragma once

#include "Defines.h"
#include "Gwen/Controls/Base.h"
#include "FloatUpDown.h"

class Float3UpDown : public Gwen::Controls::Base
{
private:
	Gwen::Controls::Label* _xLabel;
	FloatUpDown* _xUpDown;

	Gwen::Controls::Label* _yLabel;
	FloatUpDown* _yUpDown;

	Gwen::Controls::Label* _zLabel;
	FloatUpDown* _zUpDown;

	virtual void OnUpDownChanged(Gwen::Controls::Base* control );

public:	
	Float3UpDown(Gwen::Controls::Base *parent);
	~Float3UpDown();

	XMFLOAT3 GetMin();
	void SetMin(const XMFLOAT3& min);

	XMFLOAT3 GetMax();
	void SetMax(const XMFLOAT3& max);

	XMFLOAT3 GetValue();
	void SetValue(const XMFLOAT3& val);

	Gwen::Event::Caller onChanged;
};