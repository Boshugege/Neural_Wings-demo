#pragma once
#include "raylib.h"
#include "raymath.h"
#include "Engine/Math/Math.h"
class GameObject;
class mCamera
{
public:
    mCamera();
    mCamera(Vector3f pos, Vector3f tar, Vector3f u, float f,
            float nearPlane,
            float farPlane,
            const CameraMode &mode = CAMERA_CUSTOM);
    void MountTo(GameObject *target, Vector3f posOffset, Vector3f lookAtOffset);
    void Unmount();
    void SetMountIntent(const std::string &name, Vector3f posOff, Vector3f lookAtOff);
    void SetMountTarget(GameObject *target);
    const std::string &GetMountTargetName() const;
    GameObject *GetMountTarget() const;

    Vector3f Right() const;
    Vector3f Direction() const;
    Vector3f Position() const;
    Vector3f Target() const;
    Vector3f Up() const;
    float Fovy() const;

    void setRight(Vector3f right);
    void setDirection(Vector3f dir);
    void setPosition(Vector3f pos);
    void setTarget(Vector3f tar);
    void setUp(Vector3f u);
    void setFovy(float fovy);
    void setProjection(int projection);
    void setNearPlane(float nearPlane);
    void setFarPlane(float farPlane);

    Vector3f getPosition() const;
    Vector3f getDirection() const;
    Vector3f getLocalLookAtOffset() const;

    float getNearPlane() const;
    float getFarPlane() const;

    void UpdateFromDirection(Vector3f pos, Vector3f dir, Vector3f u, const CameraMode &mode = CAMERA_CUSTOM);
    void UpdateFromTarget(Vector3f pos, Vector3f tar, Vector3f u, const CameraMode &mode = CAMERA_CUSTOM);
    void UpdateFixed(Vector3f dir, Vector3f u, const CameraMode &mode = CAMERA_CUSTOM);
    void UpdatemCamera(const CameraMode &mode = CAMERA_CUSTOM);
    void Rotate(float LookHorizontal, float LookVertical);

    Camera3D &GetRawCamera();
    const Camera3D &GetConstRawCamera() const;

private:
    Camera3D m_rawCamera;

    std::string m_mountTargetName = "";
    GameObject *m_mountTarget = nullptr;
    Vector3f m_localPositionOffset = Vector3f::ZERO;
    Vector3f m_localLookAtOffset = Vector3f(0, 0, 1);

    float m_nearPlane = 0.1;
    float m_farPlane = 1000;
    Vector3f m_right = Vector3f(-1, 0, 0);
    Vector3f m_direction = Vector3f(0, 0, 1);
    Vector3f m_position = Vector3f(0, 0, 0);
    Vector3f m_target = Vector3f(0, 0, 2);
    Vector3f m_up = Vector3f(0, 1, 0);
    float m_fovy = 45;
    int m_projection = 0; // 0:透视 1:正交
};