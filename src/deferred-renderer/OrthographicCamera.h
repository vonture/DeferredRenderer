#pragma once

#include "PCH.h"
#include "Camera.h"

class OrthographicCamera : public Camera
{
private:
    float _xMin;
    float _xMax;
    float _yMin;
    float _yMax;

protected:
    void BuildProjection(XMMATRIX* outProj, float nearClip, float farClip);

public:
    OrthographicCamera();
    OrthographicCamera(float nearClip, float farClip, float minX, float minY, float maxX, float maxY);
    virtual ~OrthographicCamera();

    float GetMinX() const;
    void SetMinX(float minX);

    float GetMinY() const;
    void SetMinY(float minY);

    float GetMaxX() const;
    void SetMaxX(float maxX);

    float GetMaxY() const;
    void SetMaxY(float maxY);

    void SetBounds(float minX, float minY, float maxX, float maxY);
};