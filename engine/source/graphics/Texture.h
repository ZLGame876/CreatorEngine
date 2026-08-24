#pragma once

#include <GL/glew.h>
#include <string>

namespace eng
{
    class Texture2D
    {
    public:
        Texture2D() = default;
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        bool LoadFromFile(const std::string& path);
        void Bind(GLenum textureUnit = 0) const;
        void Unbind() const;

        // 从内存数据创建纹理（RGBA8）
        void CreateFromData(int width, int height, const unsigned char* data);

        GLuint GetID() const { return m_TextureID; }
        int GetWidth()  const { return m_Width; }
        int GetHeight() const { return m_Height; }

        bool IsValid() const { return m_TextureID != 0; }

    private:
        GLuint m_TextureID = 0;
        int m_Width = 0;
        int m_Height = 0;
    };
}