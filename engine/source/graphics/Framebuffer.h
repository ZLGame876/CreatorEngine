#pragma once

#include <GL/glew.h>

namespace eng
{
    class Framebuffer
    {
    public:
        Framebuffer() = default;
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        bool Create(int width, int height, bool withDepth = true);
        bool Resize(int width, int height);
        void Destroy();

        void Bind() const;
        static void Unbind();

        GLuint GetColorAttachment() const { return m_ColorAttachment; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        bool HasDepth() const { return m_WithDepth; }
        bool IsValid() const { return m_Framebuffer != 0 && m_ColorAttachment != 0; }

    private:
        GLuint m_Framebuffer = 0;
        GLuint m_ColorAttachment = 0;
        GLuint m_DepthStencilAttachment = 0;
        int m_Width = 0;
        int m_Height = 0;
        bool m_WithDepth = true;
    };
}
