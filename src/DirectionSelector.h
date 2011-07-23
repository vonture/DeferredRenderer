#pragma once

#include "Defines.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/RadioButton.h"

class DirectionSelector : public Gwen::Controls::Base
{
private:
	XMFLOAT2 _direction;

	bool _selected;

	Gwen::Controls::RadioButton* _outerCircle;
	Gwen::Controls::RadioButton* _innerCircle;
	
	virtual void PlaceInnerCircle();
	virtual void VerifyDirection();

	virtual void OnMouseMoved(int x, int y, int deltaX, int deltaY);
	virtual void OnMouseClickLeft( int x, int y, bool bDown);

public:
	DirectionSelector(Gwen::Controls::Base *parent);

	const XMFLOAT2& GetDirection() const { return _direction; }
	void SetDirection(const XMFLOAT2& dir) { _direction = dir; VerifyDirection(); PlaceInnerCircle(); }

	Gwen::Event::Caller onValueChanged;
};