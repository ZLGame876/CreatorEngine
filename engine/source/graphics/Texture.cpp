#include "graphics/Texture.h"
#include "core/ProjectPaths.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace eng
{
    Texture2D::~Texture2D()
    {
        if (m_TextureID != 0)
        {
            glDeleteTextures(1, &m_TextureID);
        }
    }

    bool Texture2D::LoadFromFile(const std::string& path)
    {
        // 先销毁旧纹理
        if (m_TextureID != 0)
        {
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
        }

        int channels;
        stbi_set_flip_vertically_on_load(1);  // OpenGL 纹理坐标原点在左下
        const std::filesystem::path resolvedPath = ProjectPaths::ResolveResource(path);
        unsigned char* data = stbi_load(resolvedPath.string().c_str(), &m_Width, &m_Height,
                                        &channels, 4);  // 强制 RGBA

        if (!data)
        {
            std::cerr << "无法加载纹理: " << path << " - " << stbi_failure_reason() << std::endl;
            return false;
        }

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);

        std::cout << "纹理加载成功: " << path << " (" << m_Width << "x" << m_Height << ")" << std::endl;
        return true;
    }

    void Texture2D::Bind(GLenum textureUnit) const
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
    }

    void Texture2D::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture2D::CreateFromData(int width, int height, const unsigned char* data)
    {
        if (m_TextureID != 0)
        {
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
        }

        m_Width = width;
        m_Height = height;

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}
