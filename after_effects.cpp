#include "after_effects.hpp"

#include <iostream>

namespace gl {
    AfterEffects::AfterEffects(GLsizei width, GLsizei height) : width_(width), height_(height) {
        compute_ = std::make_unique<ShaderProgram>(ShaderProgram::make_compute("shaders/glow.comp"));
        display_ = std::make_unique<ShaderProgram>(ShaderProgram::make_program("shaders/screen.vert", "shaders/screen.frag"));

        glGenTextures(3, textures_);
        glActiveTexture(GL_TEXTURE0);

        for (int i = 0; i < 3; i++) {
            glBindTexture(GL_TEXTURE_2D, textures_[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, i == 0 ? GL_RGBA8 : GL_RGBA16F, width_, height_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glGenRenderbuffers(1, &depth_buffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_, height_);

        glGenFramebuffers(1, &framebuffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures_[0], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures_[1], 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);

        const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, buffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "/!\\ AfterEffects framebuffer creation failure /!\\\n";

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glGenVertexArrays(1, &vao_);
    }

    AfterEffects::~AfterEffects() {
        glDeleteVertexArrays(1, &vao_);
        glDeleteFramebuffers(1, &framebuffer_);
        glDeleteRenderbuffers(1, &depth_buffer_);
        glDeleteTextures(3, textures_);
    }

    void AfterEffects::begin() {
        glGetIntegerv(GL_VIEWPORT, viewport_);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
        glViewport(0, 0, width_, height_);

        const GLfloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glClearBufferfv(GL_COLOR, 1, black);
    }

    void AfterEffects::apply() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        compute_->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures_[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures_[1]);
        glBindImageTexture(0, textures_[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        glDispatchCompute((width_ + 15) / 16, (height_ + 15) / 16, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]);

        display_->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures_[2]);
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }
}
