#pragma once

#include "program.hpp"

#include <memory>

namespace gl {
    class AfterEffects {
        GLuint framebuffer_ = 0;
        GLuint depth_buffer_ = 0;
        GLuint textures_[4] = {0}; // couleur, emission, flou horizontal, image finale
        GLuint vao_ = 0;

        GLsizei width_;
        GLsizei height_;
        GLint viewport_[4];

        std::unique_ptr<ShaderProgram> compute_;
        std::unique_ptr<ShaderProgram> display_;

        public:
        AfterEffects(GLsizei width, GLsizei height);
        AfterEffects(const AfterEffects&) = delete;
        AfterEffects& operator=(const AfterEffects&) = delete;
        ~AfterEffects();

        void begin();
        void apply();
    };
}
