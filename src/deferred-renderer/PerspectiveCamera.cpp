#include "PCH.h"
#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera() 
	: Camera(), _fov(1.0f), _aspect(1.0f)
{
	UpdateProjection();
}

PerspectiveCamera::PerspectiveCamera(float nearClip, float farClip, float fov, float aspect) 
	: Camera(nearClip, farClip), _fov(fov), _aspect(aspect)
{
	UpdateProjection();
}

PerspectiveCamera::~PerspectiveCamera()
{
}

void PerspectiveCamera::BuildProjection(XMMATRIX* outProj, float nearClip, float farClip)
{
	*outProj = XMMatrixPerspectiveFovLH(_fov, _aspect, nearClip, farClip);
}

void PerspectiveCamera::SetFieldOfView(float fov)
{
	_fov = fov;
	UpdateProjection();
}

float PerspectiveCamera::GetFieldOfView()
{
	return _fov;
}	

void PerspectiveCamera::SetAspectRatio(float aspect)
{
	_aspect = aspect;
	UpdateProjection();
}

float PerspectiveCamera::GetAspectRatio()
{
	return _aspect;
}