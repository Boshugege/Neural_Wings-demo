#pragma once
#include "Engine/Core/Events/IEvent.h"
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Math/Math.h"
struct CollisionEvent : public IEvent
{
  EVENT_TYPE(CollisionEvent)

  GameObject *m_object1;
  GameObject *m_object2;
  Vector3f normal;
  float penetration;
  Vector3f hitpoint;

  CollisionEvent(GameObject *object1, GameObject *object2, Vector3f normal, float penetration, Vector3f hitpoint)
      : m_object1(object1),
        m_object2(object2),
        normal(normal),
        penetration(penetration),
        hitpoint(hitpoint) {}
};