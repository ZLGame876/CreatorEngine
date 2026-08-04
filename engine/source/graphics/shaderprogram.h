#pragma once

#include <GL/glew.h>
#include <string>
#include <unordered_map>

namespace eng
{
    class ShaderProgram
    {
        private:
            std::unordered_map<std::string, GLint>m_UniformLocationCache;
            GLuint m_ShaderProgramID = 0;


        public:
           void Bind();
           GLint GetUniformLocation(const std::string& name);
    };
}