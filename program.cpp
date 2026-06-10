#include "program.hh"
#include "vector.hh"

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
                                                        fragment_id_(obj.fragment_id_), state_(obj.state_) {
        obj.id_ = 0;
        obj.vertex_id_ = 0;
        obj.fragment_id_ = 0;
        obj.state_ = false;
    }

    ShaderProgram::~ShaderProgram() {
        if (!id_) return;

        glDetachShader(id_, vertex_id_);
        glDetachShader(id_, fragment_id_);
        glDeleteShader(vertex_id_);
        glDeleteShader(fragment_id_);
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


    ShaderProgram ShaderProgram::make_program(const std::string& vertex_src, const std::string& fragment_src) {
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
            glDeleteProgram(id);

            return program;
        } 

        program.vertex_id_ = vertex_id;
        program.fragment_id_ = fragment_id;
        program.state_ = true;
        return program;
    }

    void ShaderProgram::use() {
        glUseProgram(this->id_);
    }

    void ShaderProgram::init_POV(Camera camera) {
        GLint proj_id, model_id;

        model_id = glGetUniformLocation(this->id_, "model_view_matrix");
        proj_id = glGetUniformLocation(this->id_, "projection_matrix");

        glUniformMatrix4fv(model_id, 1, GL_TRUE, camera.model_view.data());
        glUniformMatrix4fv(proj_id, 1, GL_TRUE, camera.projection.data());
    }

}
