#include "graphics/Framebuffer.h"

#include <iostream>

namespace eng
{
    Framebuffer::~Framebuffer()
    {
        Destroy();
    }

    bool Framebuffer::Create(int width, int height, bool withDepth)
    {
        if (width <= 0 || height <= 0)
        {
            return false;
        }

        Destroy();
        m_Width = width;
        m_Height = height;
        m_WithDepth = withDepth;

        glGenFramebuffers(1, &m_Framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

        glGenTextures(1, &m_ColorAttachment);
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, m_ColorAttachment, 0);

        if (withDepth)
        {
            glGenRenderbuffers(1, &m_DepthStencilAttachment);
            glBindRenderbuffer(GL_RENDERBUFFER, m_DepthStencilAttachment);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                      GL_RENDERBUFFER, m_DepthStencilAttachment);
        }

        const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) ==
                              GL_FRAMEBUFFER_COMPLETE;
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (!complete)
        {
            std::cerr << "Framebuffer is incomplete" << std::endl;
            Destroy();
        }
        return complete;
    }

    bool Framebuffer::Resize(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return false;
        }
        if (IsValid() && width == m_Width && height == m_Height)
        {
            return true;
        }
        return Create(width, height, m_WithDepth);
    }

    void Framebuffer::Destroy()
    {
        if (m_DepthStencilAttachment != 0)
        {
            glDeleteRenderbuffers(1, &m_DepthStencilAttachment);
            m_DepthStencilAttachment = 0;
        }
        if (m_ColorAttachment != 0)
        {
            glDeleteTextures(1, &m_ColorAttachment);
            m_ColorAttachment = 0;
        }
        if (m_Framebuffer != 0)
        {
            glDeleteFramebuffers(1, &m_Framebuffer);
            m_Framebuffer = 0;
        }
        m_Width = 0;
        m_Height = 0;
    }

    void Framebuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
        glViewport(0, 0, m_Width, m_Height);
    }

    void Framebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
