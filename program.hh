#pragma once

#include "camera.hh"

#include <string>
#include <vector>

#include <GL/glew.h>

namespace gl {
    class Program {

        GLuint id_;
        GLuint vertex_id_;
        GLuint fragment_id_;

        GLuint vao_id_;

        bool state_;

        Camera camera_;

        Program(GLuint id) : id_(id), state_(false) {}
        ~Program();

        public:
        
        static GLuint compile_shader(const std::string& src);
        static Program make_program(const std::string& vertex_src, const std::string& framgent_src);

        GLuint getId() { return id_; }

        char* get_log();
        bool is_ready() { return state_; }
        
        void use();

        void init_object();
        void init_POV();

    };
}