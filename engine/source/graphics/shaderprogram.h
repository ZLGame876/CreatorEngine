#pragma once

#include <GL/glew.h>
#include <string>
#include <unordered_map>

namespace eng
{
    class ShaderProgram
    {
    public:
        ShaderProgram() = default;
        ~ShaderProgram();

        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        // 从文件加载顶点着色器和片元着色器，编译并链接
        bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

        void Bind() const;
        void Unbind() const;

        GLuint GetID() const { return m_ShaderProgramID; }

        // uniform 设置
        void SetVec3(const std::string& name, float x, float y, float z);
        void SetFloat(const std::string& name, float value);
        void SetMat4(const std::string& name, const GLfloat* value);

    private:
        GLuint m_ShaderProgramID = 0;
        std::unordered_map<std::string, GLint> m_UniformLocationCache;

        GLint GetUniformLocation(const std::string& name);
        GLuint CompileShader(GLenum type, const std::string& source);
        std::string ReadFile(const std::string& path);
        bool CheckCompileErrors(GLuint shader, GLuint type);
    };
}
