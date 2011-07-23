#include "DirectionSelector.h"

DirectionSelector::DirectionSelector(Gwen::Controls::Base *parent)
	: Gwen::Controls::Base(parent), _direction(0.0f, 0.0f), _selected(false)
{
	_outerCircle = new Gwen::Controls::RadioButton(this);
	_outerCircle->SetChecked(false);
	_outerCircle->SetTabable(false);
	_outerCircle->SetDisabled(true);
	_outerCircle->SetMouseInputEnabled(false);
	_outerCircle->SetKeyboardInputEnabled(false);
	_outerCircle->Dock(Gwen::Pos::Fill);

	_innerCircle = new Gwen::Controls::RadioButton(this);	
	_innerCircle->SetChecked(true);
	_innerCircle->SetTabable(false);
	_innerCircle->SetDisabled(true);
	_innerCircle->SetMouseInputEnabled(false);
	_innerCircle->SetKeyboardInputEnabled(false);
}

void DirectionSelector::PlaceInnerCircle()
{
	Gwen::Rect size = GetBounds();
	Gwen::Rect innerSize = _innerCircle->GetBounds();

	int x = (size.w / 2) + (_direction.x * (size.w / 2));
	int y = (size.h / 2) + (_direction.y * (size.h / 2));
	
	_innerCircle->SetBounds(x - (innerSize.w / 2), y - (innerSize.h / 2), innerSize.w, innerSize.h);
}

void DirectionSelector::VerifyDirection()
{
	if (_direction.x * _direction.x + _direction.y * _direction.y > 1.0f ||
		(_direction.x == 0.0f && _direction.y == 0.0f))
	{
		XMVECTOR vec = XMLoadFloat2(&_direction);
		vec = XMVector2Normalize(vec);
		XMStoreFloat2(&_direction, vec);
	}
}

void DirectionSelector::OnMouseMoved(int x, int y, int deltaX, int deltaY)
{
	if (_selected)
	{
		Gwen::Rect size = GetBounds();		
		Gwen::Point pos = CanvasPosToLocal(Gwen::Point(x, y));
				
		// calculate direction
		_direction = XMFLOAT2((pos.x - (size.w / 2)) / (size.w / 2.0f), (pos.y - (size.h / 2)) / (size.h / 2.0f));
		VerifyDirection();
		PlaceInnerCircle();

		onValueChanged.Call(this);
	}
}

void DirectionSelector::OnMouseClickLeft(int x, int y, bool bDown)
{
	_selected = bDown;

	// Make sure it catches this point if the mouse doesn't move while clicking
	if (bDown)
	{
		OnMouseMoved(x, y, 0, 0);
	}
}