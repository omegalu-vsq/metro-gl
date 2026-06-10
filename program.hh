#pragma once

#include "camera.hh"

#include <string>
#include <vector>

#include <GL/glew.h>

namespace gl {
    class ShaderProgram {

        GLuint id_;
        GLuint vertex_id_;
        GLuint fragment_id_;

        bool state_;

        ShaderProgram(GLuint id) : id_(id), vertex_id_(0), fragment_id_(0), state_(false) {}

        public:
        ShaderProgram(const ShaderProgram& obj) = delete;

        ShaderProgram(ShaderProgram&& obj);
        ~ShaderProgram();
        
        static ShaderProgram make_program(const std::string& vertex_src, const std::string& framgent_src);

        GLuint getId() { return id_; }

        char* get_log();
        bool is_ready() { return state_; }
        
        void use();

        void init_POV(Camera camera);

    };
}
