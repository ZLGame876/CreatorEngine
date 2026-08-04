#include "graphics/shaderprogram.h"

namespace eng
{
    void ShaderProgram::Bind()
    {
        glUseProgram(m_ShaderProgramID);

    }

    GLint ShaderProgram::GetUniformLocation(const std::string& name)
    {
        auto it = m_UniformLocationCache.find(name);
        if (it != m_UniformLocationCache.end())
        {
            return it->second;
        }
        return -1;
    }
}