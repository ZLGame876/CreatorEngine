#include "core/Component.h"
#include "core/GameObject.h"

namespace eng
{
    Transform* Component::GetTransform() const
    {
        return m_GameObject ? m_GameObject->GetTransform() : nullptr;
    }
}