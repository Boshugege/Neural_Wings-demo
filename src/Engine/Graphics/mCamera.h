#include "raylib.h"
#include "raymath.h"
class mCamera : public Camera3D
{
public:
    Vector3 right;
    Vector3 direction;
    void UpdateFromDirection(Vector3 pos, Vector3 dir, Vector3 u, const CameraMode &mode = CAMERA_CUSTOM);
    void UpdateFromTarget(Vector3 pos, Vector3 tar, Vector3 u, const CameraMode &mode = CAMERA_CUSTOM);
    void Rotate(float LookHorizontal, float LookVertical);
    // TODO:挂载脚本
};