#include "graphics/shaderprogram.h"
#include "core/ProjectPaths.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace eng
{
    ShaderProgram::~ShaderProgram()
    {
        if (m_ShaderProgramID != 0)
        {
            glDeleteProgram(m_ShaderProgramID);
        }
    }

    std::string ShaderProgram::ReadFile(const std::string& path)
    {
        const std::filesystem::path resolvedPath = ProjectPaths::ResolveResource(path);
        std::ifstream file(resolvedPath);
        if (!file.is_open())
        {
            std::cerr << "无法打开着色器文件: " << path << std::endl;
            return "";
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    GLuint ShaderProgram::CompileShader(GLenum type, const std::string& source)
    {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        if (!CheckCompileErrors(shader, type))
        {
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    bool ShaderProgram::CheckCompileErrors(GLuint shader, GLuint type)
    {
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "着色器编译失败 [" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "]:" << std::endl;
            std::cerr << infoLog << std::endl;
            return false;
        }
        return true;
    }

    bool ShaderProgram::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::string vertexSource = ReadFile(vertexPath);
        std::string fragmentSource = ReadFile(fragmentPath);
        if (vertexSource.empty() || fragmentSource.empty())
        {
            return false;
        }

        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (vertexShader == 0 || fragmentShader == 0)
        {
            return false;
        }

        m_ShaderProgramID = glCreateProgram();
        glAttachShader(m_ShaderProgramID, vertexShader);
        glAttachShader(m_ShaderProgramID, fragmentShader);
        glLinkProgram(m_ShaderProgramID);

        GLint success;
        glGetProgramiv(m_ShaderProgramID, GL_LINK_STATUS, &success);
        if (!success)
        {
            GLchar infoLog[1024];
            glGetProgramInfoLog(m_ShaderProgramID, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "着色器程序链接失败:" << std::endl;
            std::cerr << infoLog << std::endl;
            glDeleteProgram(m_ShaderProgramID);
            m_ShaderProgramID = 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return m_ShaderProgramID != 0;
    }

    void ShaderProgram::Bind() const
    {
        glUseProgram(m_ShaderProgramID);
    }

    void ShaderProgram::Unbind() const
    {
        glUseProgram(0);
    }

    GLint ShaderProgram::GetUniformLocation(const std::string& name)
    {
        auto it = m_UniformLocationCache.find(name);
        if (it != m_UniformLocationCache.end())
        {
            return it->second;
        }
        GLint location = glGetUniformLocation(m_ShaderProgramID, name.c_str());
        m_UniformLocationCache[name] = location;
        return location;
    }

    void ShaderProgram::SetVec3(const std::string& name, float x, float y, float z)
    {
        GLint loc = GetUniformLocation(name);
        if (loc != -1)
        {
            glUniform3f(loc, x, y, z);
        }
    }

    void ShaderProgram::SetFloat(const std::string& name, float value)
    {
        GLint loc = GetUniformLocation(name);
        if (loc != -1)
        {
            glUniform1f(loc, value);
        }
    }

    void ShaderProgram::SetMat4(const std::string& name, const GLfloat* value)
    {
        GLint loc = GetUniformLocation(name);
        if (loc != -1)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, value);
        }
    }
}
