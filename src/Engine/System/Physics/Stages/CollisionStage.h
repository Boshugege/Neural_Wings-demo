#include "Engine/System/Physics/IPhysicsStage.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/Math/Math.h"
class GameWorld;

struct OBB
{
    Vector3f center;
    Vector3f axes[3];
    Vector3f halfExtents;
    OBB(const TransformComponent &tf, const RigidbodyComponent &rb)
    {
        this->center = tf.position;
        Vector3f localMin = rb.localBoundingBox.first;
        Vector3f localMax = rb.localBoundingBox.second;
        Vector3f size = localMax - localMin; // hitbox size
        this->halfExtents = size * 0.5f;

        Matrix3f rotMat = tf.rotation.toMatrix();
        for (int i = 0; i < 3; i++)
            axes[i] = rotMat.getCol(i);
    }
    static float GetProjectionRadius(const OBB &a, const Vector3f &axis)
    {
        float res = 0.0f;
        for (int i = 0; i < 3; i++)
            res += a.halfExtents[i] * fabsf(a.axes[i] * axis);
        return res;
    }
    static bool TestAxis(const OBB &a, const OBB &b, Vector3f axis,
                         Vector3f &outNormal, float &outMinPenetration,int curAxisIndex,int &outBestIndex)
    {
        if (axis.LengthSquared() < 0.0001f)
            return false;
        axis.Normalize();

        float distProj = fabsf((b.center - a.center) * axis);
        float sumRadius = GetProjectionRadius(a, axis) + GetProjectionRadius(b, axis);

        float pen = sumRadius - distProj;
        if (pen <= 0.0f)
            return true; // 分离
        if (pen < outMinPenetration)
        {
            outMinPenetration = pen;
            outNormal = axis;
            outBestIndex = curAxisIndex;
        }
        return false;
    }
    static bool GetCollisionInfo(const OBB &a, const OBB &b,
                                 Vector3f &normal, float &penetration, Vector3f &hitPoint)
    {
        float minPenetration = std::numeric_limits<float>::max();
        Vector3f bestAxis;
        int bestAxisIndex = -1;
        // 0-2:face A
        // 3-5:face B
        // 6-14:edge
        for (int i = 0; i < 3; i++)
            if (TestAxis(a, b, a.axes[i], bestAxis, minPenetration,i,bestAxisIndex))
                return false;
        for (int i = 0; i < 3; i++)
            if (TestAxis(a, b, b.axes[i], bestAxis, minPenetration,i+3,bestAxisIndex))
                return false;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (TestAxis(a, b, a.axes[i] ^ b.axes[j], bestAxis, minPenetration,6+i*3+j,bestAxisIndex))
                    return false;
        penetration = minPenetration;
        normal = bestAxis;
        if (normal * (b.center - a.center) < 0)
            normal = -normal;
        
        if(bestAxisIndex<6)
        {
            const OBB* refBox;
            const OBB* otherBox;
            Vector3f serachDir;
            if(bestAxisIndex<3)
            {
                refBox = &a;
                otherBox = &b;
                // B上接触点方向-normal
                serachDir = -normal;
            }
            else
            {
                refBox = &b;
                otherBox = &a;
                serachDir = normal;
            }
            hitPoint = OBB::GetSupportPoint(*otherBox,serachDir);
        }
        else
        {
            int i = (bestAxisIndex-6)/3;
            int j = (bestAxisIndex-6)%3;
            auto eDirA=a.axes[i];
            auto eDirB=b.axes[j];
            
            auto ptA=OBB::GetSupportPoint(a,normal);
            auto ptB=OBB::GetSupportPoint(b,-normal);

            auto p1=ptA+eDirA*a.halfExtents[i];
            auto q1=ptA-eDirA*a.halfExtents[i];

            auto p2=ptB+eDirB*b.halfExtents[j];
            auto q2=ptB-eDirB*b.halfExtents[j];
            hitPoint=OBB::GetContactPointEdgeEdge(p1,q1,p2,q2);
        }
        return true;
    }

    static Vector3f GetSupportPoint(const OBB &a, const Vector3f &dir)
    {
        Vector3f res = a.center;
        for (int i = 0; i < 3; i++)
        {
            float sign = dir * a.axes[i] < 0 ? -1.0f : 1.0f;
            res += a.axes[i] * sign * a.halfExtents[i];
        }
        return res;
    }
    static Vector3f GetContactPointEdgeEdge(const Vector3f &p1, const Vector3f &q1, const Vector3f &p2, const Vector3f &q2)
    {
        auto d1 = q1 - p1;
        auto d2 = q2 - p2;
        auto r = p1 - p2;

        float a = d1 * d1;
        float e = d2 * d2;
        float f = d2 * r;
        float c = d1 * r;
        float b = d1 * d2;
        float denom = a * e - b * b;

        float t1, t2;
        if (fabsf(denom) < 0.0001f)
        {
            t1 = 0.0f;
            t2 = f / e;
        }
        else
        {
            t1 = (b * f - c * e) / denom;
            t2 = (b * t1 + f) / e;
        }
        t1 = std::max(0.0f, std::min(1.0f, t1));
        t2 = std::max(0.0f, std::min(1.0f, t2));
        auto closetA = p1 + d1 * t1;
        auto closetB = p2 + d2 * t2;
        return (closetA + closetB) * 0.5f;
    }
};
class CollisionStage : public IPhysicsStage
{
public:
    CollisionStage(float eps = 0.0001f);

    void Execute(GameWorld &world, float fixedDeltaTime) override;
     void ResolveCollision(GameObject *a, GameObject *b, const Vector3f &normal, float penetration, const Vector3f &hitPoint);

private:
    float epsilon = 0.0001f;
};