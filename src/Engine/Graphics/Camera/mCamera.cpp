#include "mCamera.h"
#include "Engine/Core/GameObject/GameObject.h"
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
    if (m_mountTarget != nullptr)
    {
        auto &tf = m_mountTarget->GetComponent<TransformComponent>();
        Matrix4f mountTarWorldMat = tf.GetWorldMatrix();
        Quat4f worldRot = tf.GetWorldRotation();
        m_position = (mountTarWorldMat * Vector4f(m_localPositionOffset, 1.0f)).xyz();
        m_direction = (worldRot * m_localDirection).Normalized();
        m_up = (worldRot * m_localUp).Normalized();
        m_right = (m_direction ^ m_up).Normalized();
        m_up = (m_right ^ m_direction).Normalized();

        m_target = m_position + m_direction * 10.0f;
    }
    else
    {
        m_position = pos;
        m_direction = dir.Normalized();
        m_up = u.Normalized();
        m_right = m_direction ^ m_up;
        m_target = m_position + 10.0 * m_direction;
    }
    // m_right = (m_direction ^ m_up).Normalized();
    // m_up = (m_right ^ m_direction).Normalized();
    // m_target = m_position + m_direction * 10.0f;
    UpdatemCamera(mode);
}
void mCamera::UpdateFromTarget(Vector3f pos, Vector3f tar, Vector3f u, const CameraMode &mode)
{
    if (m_mountTarget != nullptr)
    {
        auto &tf = m_mountTarget->GetComponent<TransformComponent>();
        Matrix4f mountTarWorldMat = tf.GetWorldMatrix();
        m_position = (mountTarWorldMat * Vector4f(m_localPositionOffset, 1)).xyz();

        m_target = tar;
        m_direction = (m_target - m_position).Normalized();
        Quat4f worldRot = tf.GetWorldRotation();
        m_up = worldRot * (m_localUp);
        m_right = (m_direction ^ m_up).Normalized();
        m_up = (m_right ^ m_direction).Normalized();
    }
    else
    {
        m_position = pos;
        m_target = tar;
        m_direction = (m_target - m_position).Normalized();
        m_up = u.Normalized();
        m_right = m_direction ^ m_up;
    }
    UpdatemCamera(mode);
}

void mCamera::UpdateFixed(Vector3f dir, Vector3f u, const CameraMode &mode)
{
    if (m_mountTarget == nullptr)
    {
        std::cerr << "[mCamera]Camera not mounted to any object" << std::endl;
        return;
    }
    auto &tf = m_mountTarget->GetComponent<TransformComponent>();
    Matrix4f mountTarWorldMat = tf.GetWorldMatrix();
    m_position = (mountTarWorldMat * Vector4f(m_localPositionOffset, 1)).xyz();
    Quat4f worldRot = tf.GetWorldRotation();
    m_direction = (worldRot * dir).Normalized();
    m_up = (worldRot * u).Normalized();
    m_right = (m_direction ^ m_up).Normalized();
    m_target = m_position + m_direction;

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
void mCamera::MountTo(GameObject *target, Vector3f posOffset, Vector3f lookAtOffset)
{
    m_mountTarget = target;
    m_localPositionOffset = posOffset;
    m_localDirection = lookAtOffset;
}
void mCamera::Unmount()
{
    m_mountTarget = nullptr;

    m_localPositionOffset = Vector3f::ZERO;
    m_localDirection = Vector3f::FORWARD;
}

void mCamera::SetMountIntent(const std::string &name, Vector3f posOff, Vector3f lookAtOffset)
{
    m_mountTargetName = name;
    m_localPositionOffset = posOff;
    m_localDirection = lookAtOffset;
}
void mCamera::SetMountTarget(GameObject *target)
{
    m_mountTarget = target;
    auto &tf = m_mountTarget->GetComponent<TransformComponent>();
    Matrix4f mountTarWorldMat = tf.GetWorldMatrix();
    m_direction = (mountTarWorldMat * Vector4f(m_localDirection, 0)).xyz().Normalized();
    m_up = (mountTarWorldMat * Vector4f(m_localUp, 0)).xyz().Normalized();
    m_right = (m_direction ^ m_up).Normalized();
}
const std::string &mCamera::GetMountTargetName() const
{
    return m_mountTargetName;
}
GameObject *mCamera::GetMountTarget() const
{
    return m_mountTarget;
}

Vector3f mCamera::getPosition() const
{
    return m_position;
}
Vector3f mCamera::getDirection() const
{
    return m_direction;
}

Vector3f mCamera::getLocalLookAtOffset() const
{
    return m_localDirection;
}

void mCamera::setLocalPosition(Vector3f pos)
{
    m_localPositionOffset = pos;
}
Vector3f mCamera::getLocalPosition() const
{
    return m_localPositionOffset;
}

void mCamera::Rotate(float lookHorizontal, float lookVertical, Vector3f fixedUp)
{
    if (m_mountTarget != nullptr)
    {
        Vector3f localFixedUp = Vector3f::UP;
        m_localDirection.RotateByAxixAngle(localFixedUp, lookHorizontal);
        m_localRight = (m_localDirection ^ localFixedUp).Normalized();

        Vector3f nextDir = m_localDirection;
        nextDir.RotateByAxixAngle(m_localRight, lookVertical);
        if (abs(nextDir * localFixedUp) < 0.99f)
        {
            m_localDirection = nextDir;
        }

        m_localUp = (m_localRight ^ m_localDirection).Normalized();
        m_localDirection.Normalize();
    }
    else
    {
        if (fixedUp.Length() > 0.5)
        {
            m_direction.RotateByAxixAngle(fixedUp, lookHorizontal);
            m_right = (m_direction ^ fixedUp).Normalized();

            Vector3f nextDir = m_localDirection;
            nextDir.RotateByAxixAngle(m_right, lookVertical);
            if (abs(nextDir * fixedUp) < 0.99f)
            {
                m_localDirection = nextDir;
            }
            m_up = (m_right ^ m_direction).Normalized();
        }
        else
        {
            m_direction.RotateByAxixAngle(m_up, lookHorizontal);
            m_right.RotateByAxixAngle(m_up, lookHorizontal);

            m_direction.RotateByAxixAngle(m_right, lookVertical);
            m_up.RotateByAxixAngle(m_right, lookVertical);
            m_direction.Normalize();

            m_up.Normalize();
            m_right = (m_direction ^ m_up).Normalized();
        }
    }
    UpdateFromDirection(m_position, m_direction, m_up);
}