#pragma once

#include "camera.hpp"

#include <string>
#include <vector>

#include <GL/glew.h>

namespace gl {
    class ShaderProgram {

        GLuint id_;
        GLuint vertex_id_;
        GLuint fragment_id_;
        GLuint geometry_id_;
        GLuint tess_control_id_;
        GLuint tess_eval_id_;
        GLuint compute_id_;

        bool state_;

        ShaderProgram(GLuint id) : id_(id), vertex_id_(0), fragment_id_(0), geometry_id_(0), tess_control_id_(0), tess_eval_id_(0), compute_id_(0), state_(false) {}

        public:
        ShaderProgram(const ShaderProgram& obj) = delete;

        ShaderProgram(ShaderProgram&& obj);
        ~ShaderProgram();
        
        static ShaderProgram make_program(const std::string& vertex_src, const std::string& fragment_src, const std::string& geometry_src = "", const std::string& tess_control_src = "", const std::string& tess_eval_src = "");
        static ShaderProgram make_compute(const std::string& compute_src);

        GLuint getId() { return id_; }

        char* get_log();
        bool is_ready() { return state_; }
        bool has_tessellation() const { return tess_eval_id_ != 0; }
        
        void use();

        void init_POV(Camera camera);

    };
}
