#include "mCamera.h"
mCamera::mCamera()
{
    m_position = Vector3f(0.0f, 0.0f, 0.0f);
    m_target = Vector3f(1.0f, 0.0f, 0.0f);
    m_direction = Vector3f(1.0f, 0.0f, 0.0f);
    m_up = Vector3f(0.0f, 1.0f, 0.0f);
    m_right = Vector3f(0.0f, 0.0f, -1.0f);
    m_nearPlane = 0.01f;
    m_farPlane = 100000.0f;
    m_fovy = 1.0f;
    UpdatemCamera(CameraMode::CAMERA_CUSTOM);
}

Camera3D &mCamera::GetRawCamera()
{
    return m_rawCamera;
}
const Camera3D &mCamera::GetConstRawCamera() const
{
    return m_rawCamera;
}
mCamera::mCamera(Vector3f pos, Vector3f tar, Vector3f u, float f, float nearPlane,
                 float farPlane, const CameraMode &mode)
{
    m_position = pos;
    m_target = tar;
    m_direction = (m_target - m_position).Normalized();
    m_up = u.Normalized();
    m_right = m_direction ^ m_up;
    m_fovy = f;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    UpdatemCamera(mode);
}
Vector3f mCamera::Right() const
{
    return m_right;
}
Vector3f mCamera::Up() const
{
    return m_up;
}
Vector3f mCamera::Direction() const
{
    return m_direction;
}
Vector3f mCamera::Position() const
{
    return m_position;
}
Vector3f mCamera::Target() const
{
    return m_target;
}
float mCamera::Fovy() const
{
    return m_fovy;
}

void mCamera::setRight(Vector3f right)
{
    m_right = right;
}
void mCamera::setDirection(Vector3f dir)
{
    m_direction = dir;
}
void mCamera::setPosition(Vector3f pos)
{
    m_position = pos;
}
void mCamera::setTarget(Vector3f tar)
{
    m_target = tar;
}
void mCamera::setUp(Vector3f u)
{
    m_up = u;
}
void mCamera::setFovy(float fovy)
{
    m_fovy = fovy;
}
void mCamera::setProjection(int p)
{
    m_projection = p;
}
void mCamera::setNearPlane(float nearPlane)
{
    m_nearPlane = nearPlane;
}
void mCamera::setFarPlane(float farPlane)
{
    m_farPlane = farPlane;
}

float mCamera::getNearPlane() const
{
    return m_nearPlane;
}
float mCamera::getFarPlane() const
{
    return m_farPlane;
}

void mCamera::UpdateFromDirection(Vector3f pos, Vector3f dir, Vector3f u, const CameraMode &mode)
{
    m_position = pos;
    m_direction = dir.Normalized();
    m_up = u.Normalized();
    m_right = m_direction ^ m_up;
    m_target = m_position + 10.0 * m_direction;

    UpdatemCamera(mode);
}
void mCamera::UpdateFromTarget(Vector3f pos, Vector3f tar, Vector3f u, const CameraMode &mode)
{
    m_position = pos;
    m_target = tar;
    m_direction = (m_target - m_position).Normalized();
    m_up = u.Normalized();
    m_right = m_direction ^ m_up;
    UpdatemCamera(mode);
}
void mCamera::UpdatemCamera(const CameraMode &mode)
{
    m_rawCamera.position = m_position;
    m_rawCamera.target = m_target;
    m_rawCamera.up = m_up;
    m_rawCamera.fovy = m_fovy;
    m_rawCamera.projection = m_projection;
    UpdateCamera(&m_rawCamera, mode);
}
void mCamera::Rotate(float lookHorizontal, float lookVertical)
{

    m_direction.RotateByAxixAngle(m_up, lookHorizontal);
    m_right.RotateByAxixAngle(m_up, lookHorizontal);
    m_direction.RotateByAxixAngle(m_right, lookVertical);
    m_up.RotateByAxixAngle(m_right, lookVertical);

    UpdateFromDirection(m_position, m_direction, m_up);
}