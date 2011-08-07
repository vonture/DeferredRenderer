#include "PCH.h"
#include "Float2UpDown.h"

Float2UpDown::Float2UpDown(Gwen::Controls::Base *parent)
	: Gwen::Controls::Base(parent)
{
	_xLabel = new Gwen::Controls::Label(this);
	_xLabel->SetText(" X: ");
	_xLabel->SizeToContents();
	_xLabel->Dock(Gwen::Pos::Left);

	_xUpDown = new FloatUpDown(this);
	_xUpDown->SetWidth(60);
	_xUpDown->Dock(Gwen::Pos::Left);
	_xUpDown->onChanged.Add(this, &Float2UpDown::OnUpDownChanged);

	_yLabel = new Gwen::Controls::Label(this);
	_yLabel->SetText(" Y: ");
	_yLabel->SizeToContents();
	_yLabel->Dock(Gwen::Pos::Left);

	_yUpDown = new FloatUpDown(this);
	_yUpDown->SetWidth(60);
	_yUpDown->Dock(Gwen::Pos::Left);
	_yUpDown->onChanged.Add(this, &Float2UpDown::OnUpDownChanged);
}

Float2UpDown::~Float2UpDown()
{
}

XMFLOAT2 Float2UpDown::GetMin()
{
	return XMFLOAT2(_xUpDown->GetMin(), _yUpDown->GetMin());
}

void Float2UpDown::SetMin(const XMFLOAT2& min)
{
	_xUpDown->SetMin(min.x);
	_yUpDown->SetMin(min.y);
}

XMFLOAT2 Float2UpDown::GetMax()
{
	return XMFLOAT2(_xUpDown->GetMax(), _yUpDown->GetMax());
}

void Float2UpDown::SetMax(const XMFLOAT2& max)
{	
	_xUpDown->SetMax(max.x);
	_yUpDown->SetMax(max.y);
}

XMFLOAT2 Float2UpDown::GetValue()
{
	return XMFLOAT2(_xUpDown->GetValue(), _yUpDown->GetValue());
}

void Float2UpDown::SetValue(const XMFLOAT2& val)
{
	_xUpDown->SetValue(val.x);
	_yUpDown->SetValue(val.y);
}

void Float2UpDown::OnUpDownChanged(Gwen::Controls::Base* obj)
{	
	onChanged.Call(this);
}