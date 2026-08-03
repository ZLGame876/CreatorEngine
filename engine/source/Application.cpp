#include "Application.h"

namespace Eng
{
    void Application::SetNeedsToBeClosed(bool value)
    {
        m_NeedsToBeClosed = value;
    }
    bool Application::NeedsToBeClosed() const
    {
        return m_NeedsToBeClosed;
    }
}