#include "program.hh"

#include "object.hh"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl {

    Program::~Program() {
        glDetachShader(id_, vertex_id_);
        glDetachShader(id_, fragment_id_);
        glDeleteShader(vertex_id_);
        glDeleteShader(fragment_id_);
        glDeleteProgram(id_);
    }

    char* Program::get_log() {
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

    GLuint Program::compile_shader(const std::string& src, GLenum type) {
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


    Program Program::make_program(const std::string& vertex_src, const std::string& fragment_src) {
        GLuint vertex_id = compile_shader(vertex_src, GL_VERTEX_SHADER); 
        if (!vertex_id) {
        
        }

        GLuint fragment_id = compile_shader(fragment_src, GL_FRAGMENT_SHADER); 
        if (!fragment_id) {

        }

        GLuint id = glCreateProgram();
        Program program{id};
        if (!id) {
            std::cerr << "/!\\ Program creation failure /!\\\n";

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
            std::cerr << "/!\\ Program linkage failure /!\\\n";

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

    void Program::use() {
        glUseProgram(this->id_);
    }

    void Program::init_object() {
        GLuint vbo_id;
        GLint vertex_position = glGetAttribLocation(this->id_, "position");

        if (vertex_position == -1) return; 

        glGenVertexArrays(1, &vao_id_);
        glBindVertexArray(vao_id_);

        glGenBuffers(1, &vbo_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_buffer) * sizeof(GLfloat), cube_buffer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(vertex_position);

        glBindVertexArray(0);
    }

    void Program::init_POV() {
        GLuint proj_id, model_id;

        camera_.lookat(-2.0f, -2.0f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        camera_.frustrum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);

        model_id = glGetUniformLocation(this->id_, "model_view_matrix");
        proj_id = glGetUniformLocation(this->id_, "projection_matrix");

        glUniformMatrix4fv(model_id, 1, GL_TRUE, camera_.model_view.data());
        glUniformMatrix4fv(proj_id, 1, GL_TRUE, camera_.projection.data());
    }

    void Program::display() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 1.0f, 0.1f, 1.0f);

        GLuint color_id = glGetUniformLocation(this->id_, "color");

        Vector3 color{0.8, 0.3, 0.1};
        
        glUniform3fv(color_id, 1, color.data());

        glBindVertexArray(this->vao_id_);

        glDrawArrays(GL_TRIANGLES, 0, cube_buffer.size());

        glBindVertexArray(0);
    }


}