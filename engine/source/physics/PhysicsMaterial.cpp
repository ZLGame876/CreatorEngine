#include "physics/PhysicsMaterial.h"
#include <algorithm>

namespace eng
{
    PhysicsMaterial::PhysicsMaterial(float friction, float restitution)
        : m_Friction(std::max(0.0f, friction))
        , m_Restitution(std::clamp(restitution, 0.0f, 1.0f))
    {
    }
}
