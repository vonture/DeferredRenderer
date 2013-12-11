#pragma once

#include "PCH.h"
#include "FirstPersonCamera.h"

class TestingCamera : public FirstPersonCamera
{
private:
    float _moveSpeed;
    float _rotSpeed;
    float _rollSpeed;
public:
    TestingCamera();
    TestingCamera(float nearClip, float farClip, float fov, float aspect);
    virtual ~TestingCamera();

    float GetMovementSpeed() const;
    void SetMovementSpeed(float speed);

    float GetRotationSpeed() const;
    void SetRotationSpeed(float rotSpeed);

    float GetRollSpeed() const;
    void SetRollSpeed(float val);
};