#include "PCH.h"
#include "OrthographicCamera.h"

OrthographicCamera::OrthographicCamera()
	: Camera(), _xMin(0.0f), _yMin(0.0f), _xMax(1.0f), _yMax(1.0f)
{
	UpdateProjection();
}

OrthographicCamera::OrthographicCamera(float nearClip, float farClip, float minX, float minY, float maxX, float maxY)
	: Camera(nearClip, farClip), _xMin(minX), _yMin(minY), _xMax(maxX), _yMax(maxY)
{
	UpdateProjection();
}

void OrthographicCamera::BuildProjection(XMMATRIX* outProj, float nearClip, float farClip)
{
	*outProj = XMMatrixOrthographicOffCenterLH(_xMin, _xMax, _yMin, _yMax, nearClip, farClip);
}

float OrthographicCamera::GetMinX() const
{ 
	return _xMin;
}

float OrthographicCamera::GetMinY() const
{ 
	return _yMin;
}

float OrthographicCamera::GetMaxX() const 
{ 
	return _xMax;
}

float OrthographicCamera::GetMaxY() const 
{ 
	return _yMax;
}

void OrthographicCamera::SetMinX(float minX)
{
	_xMin = minX;
	UpdateProjection();
}

void OrthographicCamera::SetMinY(float minY)
{
	_yMin = minY;
	UpdateProjection();
}

void OrthographicCamera::SetMaxX(float maxX)
{
	_xMax = maxX;
	UpdateProjection();
}

void OrthographicCamera::SetMaxY(float maxY)
{
	_yMax = maxY;
	UpdateProjection();
}

void OrthographicCamera::SetBounds(float minX, float minY, float maxX, float maxY)
{
	_xMin = minX;
	_yMin = minY;
	_xMax = maxX;
	_yMax = maxY;
	UpdateProjection();
}