#include "raylib.h"
#include "raymath.h"
#include "Engine/Math/Math.h"
class mCamera : public Camera3D
{
public:

    mCamera();
    mCamera(Vector3f pos, Vector3f tar, Vector3f u, float f, const CameraMode &mode = CAMERA_CUSTOM);
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

    void UpdateFromDirection(Vector3f pos, Vector3f dir, Vector3f u, const CameraMode &mode = CAMERA_CUSTOM);
    void UpdateFromTarget(Vector3f pos, Vector3f tar, Vector3f u, const CameraMode &mode = CAMERA_CUSTOM);
    void UpdatemCamera(const CameraMode &mode = CAMERA_CUSTOM);
    void Rotate(float LookHorizontal, float LookVertical);
    // TODO:挂载脚本
private:
    Vector3f m_right;
    Vector3f m_direction;
    Vector3f m_position;
    Vector3f m_target;
    Vector3f m_up;
    float m_fovy;
    int m_projection; // 0:透视 1:正交
};