#include "program.hpp"
#include "vector.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl {
    static const std::vector<GLfloat> cube_buffer = {
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };

    ShaderProgram::ShaderProgram(ShaderProgram&& obj) : id_(obj.id_), vertex_id_(obj.vertex_id_),
                                                        fragment_id_(obj.fragment_id_), geometry_id_(obj.geometry_id_),
                                                        tess_control_id_(obj.tess_control_id_), tess_eval_id_(obj.tess_eval_id_),
                                                        compute_id_(obj.compute_id_), state_(obj.state_) {
        obj.id_ = 0;
        obj.vertex_id_ = 0;
        obj.fragment_id_ = 0;
        obj.geometry_id_ = 0;
        obj.tess_control_id_ = 0;
        obj.tess_eval_id_ = 0;
        obj.compute_id_ = 0;
        obj.state_ = false;
    }

    ShaderProgram::~ShaderProgram() {
        if (!id_) return;

        if (vertex_id_) {
            glDetachShader(id_, vertex_id_);
            glDeleteShader(vertex_id_);
        }

        if (fragment_id_) {
            glDetachShader(id_, fragment_id_);
            glDeleteShader(fragment_id_);
        }

        if (geometry_id_) {
            glDetachShader(id_, geometry_id_);
            glDeleteShader(geometry_id_);
        }

        if (tess_control_id_) {
            glDetachShader(id_, tess_control_id_);
            glDeleteShader(tess_control_id_);
        }

        if (tess_eval_id_) {
            glDetachShader(id_, tess_eval_id_);
            glDeleteShader(tess_eval_id_);
        }

        if (compute_id_) {
            glDetachShader(id_, compute_id_);
            glDeleteShader(compute_id_);
        }

        glDeleteProgram(id_);
    }

    char* ShaderProgram::get_log() {
        int info_length = 0;
        glGetProgramiv(this->id_, GL_INFO_LOG_LENGTH, &info_length);

        GLchar* info = (GLchar*)std::malloc(info_length + 1);
        glGetProgramInfoLog(this->id_, info_length, &info_length, info);

        return info;
    }

    std::string readFile(const std::string& src) {
        std::ifstream file(src);
        std::string str;
        std::string file_contents;
        
        while (std::getline(file, str)) {
            file_contents += str;
            file_contents.push_back('\n');
        } 
        
        return file_contents;
    }

    GLuint compile_shader(const std::string& src, GLenum type) {
        std::string shader = readFile(src);

        const GLchar* files[] = { shader.c_str() };
        const GLint lengths[] = { static_cast<GLint>(shader.size()) };

        GLuint id = glCreateShader(type);
        glShaderSource(id, 1, files, lengths);
        glCompileShader(id);
        
        int compile_status = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compile_status);

        if (compile_status != GL_TRUE) {
            std::cerr << "/!\\ Shader compilation failure /!\\ (" + src + ")\n";

            int info_length = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_length);

            GLchar* info = (GLchar*)std::malloc(info_length + 1);
            glGetShaderInfoLog(id, info_length, &info_length, info);
            std::cerr << info << std::endl;
            std::free(info);

            glDeleteShader(id);
            return 0;
        }
        else {
            std::cout << "Shader infos (" + src + ")\n";

            int info_length = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_length);

            GLchar* info = (GLchar*)std::malloc(info_length + 1);
            glGetShaderInfoLog(id, info_length, &info_length, info);
            std::cerr << info << std::endl;
            std::free(info);
        }

        return id;
    }


    ShaderProgram ShaderProgram::make_program(const std::string& vertex_src, const std::string& fragment_src, const std::string& geometry_src, const std::string& tess_control_src, const std::string& tess_eval_src) {
        GLuint vertex_id = compile_shader(vertex_src, GL_VERTEX_SHADER); 
        if (!vertex_id) {
        
        }

        GLuint fragment_id = compile_shader(fragment_src, GL_FRAGMENT_SHADER); 
        if (!fragment_id) {

        }

        GLuint id = glCreateProgram();
        ShaderProgram program{id};
        if (!id) {
            std::cerr << "/!\\ ShaderProgram creation failure /!\\\n";

            auto info = program.get_log();
            std::cerr << info << std::endl;

            return program; 
        }
        
        glAttachShader(id, vertex_id);
        glAttachShader(id, fragment_id);

        GLuint geometry_id = 0;

        if (!geometry_src.empty()) {
            geometry_id = compile_shader(geometry_src, GL_GEOMETRY_SHADER);
            glAttachShader(id, geometry_id);
        }

        GLuint tess_control_id = 0;
        GLuint tess_eval_id = 0;

        if (!tess_control_src.empty()) {
            tess_control_id = compile_shader(tess_control_src, GL_TESS_CONTROL_SHADER);
            glAttachShader(id, tess_control_id);
        }

        if (!tess_eval_src.empty()) {
            tess_eval_id = compile_shader(tess_eval_src, GL_TESS_EVALUATION_SHADER);
            glAttachShader(id, tess_eval_id);
        }

        glLinkProgram(id);

        int link_status = 0;
        glGetProgramiv(id, GL_LINK_STATUS, &link_status);
        if (link_status != GL_TRUE) {
            std::cerr << "/!\\ ShaderProgram linkage failure /!\\\n";

            auto info = program.get_log();
            std::cerr << info << std::endl;

            glDetachShader(id, vertex_id);
            glDetachShader(id, fragment_id);
            glDeleteShader(vertex_id);
            glDeleteShader(fragment_id);
            if (geometry_id) {
                glDetachShader(id, geometry_id);
                glDeleteShader(geometry_id);
            }
            if (tess_control_id) {
                glDetachShader(id, tess_control_id);
                glDeleteShader(tess_control_id);
            }
            if (tess_eval_id) {
                glDetachShader(id, tess_eval_id);
                glDeleteShader(tess_eval_id);
            }
            glDeleteProgram(id);
            program.id_ = 0;

            return program;
        } 

        glUseProgram(id);
        glUniform1i(glGetUniformLocation(id, "signal_texture"), 0);
        glUniform1i(glGetUniformLocation(id, "LTC1"), 8);
        glUniform1i(glGetUniformLocation(id, "LTC2"), 9);
        glUniform1i(glGetUniformLocation(id, "shadow_map"), 10);

        program.vertex_id_ = vertex_id;
        program.fragment_id_ = fragment_id;
        program.geometry_id_ = geometry_id;
        program.tess_control_id_ = tess_control_id;
        program.tess_eval_id_ = tess_eval_id;
        program.state_ = true;
        return program;
    }

    ShaderProgram ShaderProgram::make_compute(const std::string& compute_src) {
        ShaderProgram program{glCreateProgram()};
        GLuint compute_id = compile_shader(compute_src, GL_COMPUTE_SHADER);
        if (!compute_id) return program;

        program.compute_id_ = compute_id;
        glAttachShader(program.id_, compute_id);
        glLinkProgram(program.id_);

        GLint link_status;
        glGetProgramiv(program.id_, GL_LINK_STATUS, &link_status);

        if (link_status != GL_TRUE) {
            std::cerr << "/!\\ ShaderProgram linkage failure /!\\\n";
            auto info = program.get_log();
            std::cerr << info << std::endl;
            std::free(info);
            return program;
        }

        program.state_ = true;
        return program;
    }

    void ShaderProgram::use() {
        glUseProgram(this->id_);
    }

    void ShaderProgram::init_POV(Camera camera) {
        GLint pos_id, proj_id, view_id;

        pos_id = glGetUniformLocation(this->id_, "camera_ws");
        view_id = glGetUniformLocation(this->id_, "view_matrix");
        proj_id = glGetUniformLocation(this->id_, "projection_matrix");

        glUniform3fv(pos_id, 1, camera.pos.data());
        glUniformMatrix4fv(view_id, 1, GL_TRUE, camera.view.data());
        glUniformMatrix4fv(proj_id, 1, GL_TRUE, camera.projection.data());
    }

}
