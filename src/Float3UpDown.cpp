#include "Float3UpDown.h"

Float3UpDown::Float3UpDown(Gwen::Controls::Base *parent)
	: Gwen::Controls::Base(parent)
{
	_xLabel = new Gwen::Controls::Label(this);
	_xLabel->SetText(" X: ");
	_xLabel->SizeToContents();
	_xLabel->Dock(Gwen::Pos::Left);

	_xUpDown = new FloatUpDown(this);
	_xUpDown->SetWidth(60);
	_xUpDown->Dock(Gwen::Pos::Left);
	_xUpDown->onChanged.Add(this, &Float3UpDown::OnUpDownChanged);

	_yLabel = new Gwen::Controls::Label(this);
	_yLabel->SetText(" Y: ");
	_yLabel->SizeToContents();
	_yLabel->Dock(Gwen::Pos::Left);

	_yUpDown = new FloatUpDown(this);
	_yUpDown->SetWidth(60);
	_yUpDown->Dock(Gwen::Pos::Left);
	_yUpDown->onChanged.Add(this, &Float3UpDown::OnUpDownChanged);

	_zLabel = new Gwen::Controls::Label(this);
	_zLabel->SetText(" Z: ");
	_zLabel->SizeToContents();
	_zLabel->Dock(Gwen::Pos::Left);

	_zUpDown = new FloatUpDown(this);
	_zUpDown->SetWidth(60);
	_zUpDown->Dock(Gwen::Pos::Left);
	_zUpDown->onChanged.Add(this, &Float3UpDown::OnUpDownChanged);
}

Float3UpDown::~Float3UpDown()
{
}

XMFLOAT3 Float3UpDown::GetMin()
{
	return XMFLOAT3(_xUpDown->GetMin(), _yUpDown->GetMin(), _zUpDown->GetMin());
}

void Float3UpDown::SetMin(const XMFLOAT3& min)
{
	_xUpDown->SetMin(min.x);
	_yUpDown->SetMin(min.y);
	_zUpDown->SetMin(min.z);
}

XMFLOAT3 Float3UpDown::GetMax()
{
	return XMFLOAT3(_xUpDown->GetMax(), _yUpDown->GetMax(), _zUpDown->GetMax());
}

void Float3UpDown::SetMax(const XMFLOAT3& max)
{	
	_xUpDown->SetMax(max.x);
	_yUpDown->SetMax(max.y);
	_zUpDown->SetMax(max.z);
}

XMFLOAT3 Float3UpDown::GetValue()
{
	return XMFLOAT3(_xUpDown->GetValue(), _yUpDown->GetValue(), _zUpDown->GetValue());
}

void Float3UpDown::SetValue(const XMFLOAT3& val)
{
	_xUpDown->SetValue(val.x);
	_yUpDown->SetValue(val.y);
	_zUpDown->SetValue(val.z);
}

void Float3UpDown::OnUpDownChanged(Gwen::Controls::Base* obj)
{	
	onChanged.Call(this);
}

