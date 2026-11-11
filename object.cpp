#include "light.hpp"
#include "object.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace gl {
    void flip_texture_x(unsigned char* buffer, int width, int height, int channels) {
        for (int y = 0; y < height; y++) {
            unsigned char* row = buffer + y * width * channels;
            for (int x = 0; x < width / 2; x++) {
                unsigned char* left = row + x * channels;
                unsigned char* right = row + (width - 1 - x) * channels;
                for (int c = 0; c < channels; c++)
                    std::swap(left[c], right[c]);
            }
        }
    }

    Texture::Texture(const std::string& src, bool flip_y, bool flip_x) : id_(0) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(flip_y);
        unsigned char* buffer = stbi_load(src.c_str(), &width, &height, &channels, 0);
        stbi_set_flip_vertically_on_load(false);
        if (!buffer) {
            std::cerr << "/!\\ Texture load failure: " << src << std::endl;
            return;
        }

        if (flip_x) flip_texture_x(buffer, width, height, channels);

        GLenum format = GL_RGB;
        if (channels == 4) format = GL_RGBA;
        if (channels == 1) format = GL_RED;

        glGenTextures(1, &id_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, buffer);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(buffer);
    }

    Texture::Texture(const float* src) {
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64, 64, 0, GL_RGBA, GL_FLOAT, src);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Texture::~Texture() {
        if (id_) glDeleteTextures(1, &id_);
    }

    void GLobject::init_default(Camera camera) {
        shaders_->use();
        shaders_->init_POV(camera);
    }

    void GLobject::init_lights(const std::vector<GLfloat>& lights) {
        shaders_->use();
        GLuint nb_lights = lights.size() / 3;
        GLint lights_id, nb_lights_id, light_color_id;

        lights_id = glGetUniformLocation(shaders_->getId(), "lights");
        nb_lights_id = glGetUniformLocation(shaders_->getId(), "nb_light");
        light_color_id = glGetUniformLocation(shaders_->getId(), "light_color");

        glUniform1i(nb_lights_id, nb_lights);
        
        const GLfloat light_intensity = 0.55f;
        Vector3 color{light_intensity, light_intensity, light_intensity};
        glUniform3fv(light_color_id, 1, color.data());

        glUniform3fv(lights_id, nb_lights, lights.data());
    }

    void GLobject::init_lights(const std::vector<std::unique_ptr<GLlight>>& lights) {
        shaders_->use();

        GLint nb_lights_id = glGetUniformLocation(shaders_->getId(), "nb_tubelights");
        glUniform1i(nb_lights_id, lights.size());

        for (size_t i = 0; i < lights.size(); i++)
            lights[i].get()->init_uniform(shaders_->getId(), i); 
    }

    void GLobject::init_metro_lights(GLfloat train_z, GLfloat power) {
        shaders_->use();

        GLint metro_lights_id = glGetUniformLocation(shaders_->getId(), "metro_lights");
        GLint metro_light_power_id = glGetUniformLocation(shaders_->getId(), "metro_light_power");
        if (metro_lights_id == -1) return;

        const GLfloat front_z = 39.7f + train_z;
        const GLfloat lights[] = {
            11.78f, 1.375f, front_z,
            13.08f, 1.375f, front_z
        };

        glUniform3fv(metro_lights_id, 2, lights);
        glUniform1f(metro_light_power_id, power);
    }

    void load_mesh(const aiMesh* src, std::vector<GLfloat>& vec) {
        bool has_normals = src->HasNormals();
        bool has_uv = src->HasTextureCoords(0);

        for (size_t i = 0; i < src->mNumVertices; i++) {
            vec.push_back(src->mVertices[i].x);
            vec.push_back(src->mVertices[i].y);
            vec.push_back(src->mVertices[i].z);

            if (has_normals) {
                vec.push_back(src->mNormals[i].x);
                vec.push_back(src->mNormals[i].y);
                vec.push_back(src->mNormals[i].z);
            }
            else {
                vec.push_back(0.0f);
                vec.push_back(1.0f);
                vec.push_back(0.0f);
            }

            if (has_uv) {
                vec.push_back(src->mTextureCoords[0][i].x);
                vec.push_back(src->mTextureCoords[0][i].y);
            }
            else {
                vec.push_back(0.0f);
                vec.push_back(0.0f);
            }
        }
    }

    void load_indices(const aiMesh* src, std::vector<GLuint>& vec) {
        for (size_t i = 0; i < src->mNumFaces; i++) {
            for (size_t j = 0; j < src->mFaces[i].mNumIndices; j++)
                vec.push_back(src->mFaces[i].mIndices[j]);
        }
    }

    GLobject GLobject::make_object(const aiMesh* src, std::shared_ptr<ShaderProgram> shaders) {
        GLobject object{std::move(shaders)};

        std::vector<GLfloat> buffer;
        load_mesh(src, buffer);

        std::vector<GLuint> indices;
        load_indices(src, indices);

        GLuint vbo_id;
        GLuint ebo_id;
        GLint vertex_position = glGetAttribLocation(object.shaders_->getId(), "position");
        GLint vertex_normal = glGetAttribLocation(object.shaders_->getId(), "normal");
        GLint vertex_uv = glGetAttribLocation(object.shaders_->getId(), "uv");

        if (vertex_position == -1) return object; 
        //if (vertex_normal == -1) return object; 
        //if (vertex_uv == -1) return object; 

        glGenVertexArrays(1, &(object.vao_id_));
        glBindVertexArray(object.vao_id_);

        glGenBuffers(1, &vbo_id);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(GLfloat), buffer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(vertex_position);

        if (vertex_normal != -1) {
            glVertexAttribPointer(vertex_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(vertex_normal);
        }

        if (vertex_uv != -1) {
            glVertexAttribPointer(vertex_uv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(2 * 3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(vertex_uv);
        }

        glGenBuffers(1, &ebo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

        object.state_ = true;
        object.nb_vertices_ = src->mNumVertices;
        object.nb_indices_ = indices.size();
        return object;
    }

    GLobject GLobject::make_strip(std::shared_ptr<ShaderProgram> shaders, float x0, float x1, float y, float z0, float z1) {
        GLobject object{std::move(shaders)};

        std::vector<GLfloat> buffer = {
            x0, y, z0, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            x1, y, z0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            x1, y, z1, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            x0, y, z1, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        };
        std::vector<GLuint> indices = {0, 1, 2, 0, 2, 3};

        GLuint vbo_id;
        GLuint ebo_id;
        GLint vertex_position = glGetAttribLocation(object.shaders_->getId(), "position");
        GLint vertex_normal = glGetAttribLocation(object.shaders_->getId(), "normal");
        GLint vertex_uv = glGetAttribLocation(object.shaders_->getId(), "uv");

        if (vertex_position == -1) return object;

        glGenVertexArrays(1, &(object.vao_id_));
        glBindVertexArray(object.vao_id_);

        glGenBuffers(1, &vbo_id);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(GLfloat), buffer.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_position, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(vertex_position);

        if (vertex_normal != -1) {
            glVertexAttribPointer(vertex_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(vertex_normal);
        }

        if (vertex_uv != -1) {
            glVertexAttribPointer(vertex_uv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
            glEnableVertexAttribArray(vertex_uv);
        }

        glGenBuffers(1, &ebo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

        object.state_ = true;
        object.nb_vertices_ = 4;
        object.nb_indices_ = indices.size();
        return object;
    }

    GLobject GLobject::make_cylinder(std::shared_ptr<ShaderProgram> shaders, float radius, float length) {
        const unsigned int segments = 24;
        const float pi = 3.14159265f;
        const unsigned int bottom = 4 * segments;
        const unsigned int top = bottom + 1;

        aiMesh mesh;
        mesh.mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
        mesh.mNumVertices = 4 * segments + 2;
        mesh.mNumFaces = 4 * segments;
        mesh.mVertices = new aiVector3D[mesh.mNumVertices];
        mesh.mNormals = new aiVector3D[mesh.mNumVertices];
        mesh.mFaces = new aiFace[mesh.mNumFaces];

        mesh.mVertices[bottom] = aiVector3D(0.0f, 0.0f, -length / 2.0f);
        mesh.mVertices[top] = aiVector3D(0.0f, 0.0f, length / 2.0f);
        mesh.mNormals[bottom] = aiVector3D(0.0f, 0.0f, -1.0f);
        mesh.mNormals[top] = aiVector3D(0.0f, 0.0f, 1.0f);

        for (unsigned int i = 0; i < segments; i++) {
            float angle = 2.0f * pi * i / segments;
            aiVector3D normal(std::cos(angle), std::sin(angle), 0.0f);
            mesh.mVertices[i] = aiVector3D(radius * normal.x, radius * normal.y, -length / 2.0f);
            mesh.mVertices[i + segments] = aiVector3D(radius * normal.x, radius * normal.y, length / 2.0f);
            mesh.mNormals[i] = mesh.mNormals[i + segments] = normal;

            mesh.mVertices[i + 2 * segments] = mesh.mVertices[i];
            mesh.mVertices[i + 3 * segments] = mesh.mVertices[i + segments];
            mesh.mNormals[i + 2 * segments] = mesh.mNormals[bottom];
            mesh.mNormals[i + 3 * segments] = mesh.mNormals[top];

            unsigned int next = (i + 1) % segments;
            unsigned int triangles[4][3] = {
                {i, next, i + segments},
                {next, next + segments, i + segments},
                {bottom, next + 2 * segments, i + 2 * segments},
                {top, i + 3 * segments, next + 3 * segments}
            };

            for (unsigned int j = 0; j < 4; j++) {
                aiFace& face = mesh.mFaces[4 * i + j];
                face.mNumIndices = 3;
                face.mIndices = new unsigned int[3]{triangles[j][0], triangles[j][1], triangles[j][2]};
            }
        }

        return make_object(&mesh, std::move(shaders));
    }

    void GLobject::init_material() {
        glUniform1f(glGetUniformLocation(shaders_->getId(), "roughness"), roughness_);
        glUniform1f(glGetUniformLocation(shaders_->getId(), "metallic"), metallic_);
    }

    void GLobject::draw(Vector3 color, Vector3 offset) {
        if (!state_) return;

        shaders_->use();
        init_material();
        GLint color_id = glGetUniformLocation(shaders_->getId(), "color");
        GLint offset_id = glGetUniformLocation(shaders_->getId(), "model_offset");
        GLint podo_mode_id = glGetUniformLocation(shaders_->getId(), "podotactile_mode");

        glUniform3fv(color_id, 1, color.data());
        glUniform3fv(offset_id, 1, offset.data());
        if (podo_mode_id != -1)
            glUniform1i(podo_mode_id, 0);

        glBindVertexArray(vao_id_);

        glDrawElements(GL_TRIANGLES, nb_indices_, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, nb_vertices_);

        glBindVertexArray(0);
    }

    void GLobjectPodo::draw(Vector3 color, Vector3 offset) {
        if (!state_) return;

        shaders_->use();
        init_material();
        GLint color_id = glGetUniformLocation(shaders_->getId(), "color");
        GLint offset_id = glGetUniformLocation(shaders_->getId(), "model_offset");
        GLint podo_mode_id = glGetUniformLocation(shaders_->getId(), "podotactile_mode");

        glUniform3fv(color_id, 1, color.data());
        glUniform3fv(offset_id, 1, offset.data());
        if (podo_mode_id != -1)
            glUniform1i(podo_mode_id, 1);

        glBindVertexArray(vao_id_);
        glDrawElements(GL_TRIANGLES, nb_indices_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (podo_mode_id != -1)
            glUniform1i(podo_mode_id, 0);
    }

    void GLobjectTex::draw(Vector3 color, Vector3 offset) {
        if (!state_) return;

        shaders_->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_->id());

        GLobject::draw(color, offset);
    }

    void GLobjectMetro::draw(Vector3, Vector3 offset) {
        if (!state_) return;

        if (transparent())
            glDepthMask(GL_FALSE);

        shaders_->use();
        init_material();
        GLint color_id = glGetUniformLocation(shaders_->getId(), "color");
        GLint alpha_id = glGetUniformLocation(shaders_->getId(), "alpha");
        GLint offset_id = glGetUniformLocation(shaders_->getId(), "model_offset");

        glUniform3fv(color_id, 1, color_.data());
        glUniform1f(alpha_id, alpha_);
        glUniform3fv(offset_id, 1, offset.data());

        glBindVertexArray(vao_id_);
        glDrawElements(GL_TRIANGLES, nb_indices_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (transparent())
            glDepthMask(GL_TRUE);
    }
}
