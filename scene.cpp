#include "scene.hh"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace gl {
    Texture::Texture(const std::string& src) : id_(0) {
        int width, height, channels;
        unsigned char* buffer = stbi_load(src.c_str(), &width, &height, &channels, 0);
        if (!buffer) {
            std::cerr << "Texture load failure: " << src << std::endl;
            return;
        }

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

    Texture::~Texture() {
        if (id_) glDeleteTextures(1, &id_);
    }

    void GLobject::init_default(Camera camera) {
        shaders_->use();
        shaders_->init_POV(camera);
    }

    void GLobject::init_lights() {
        shaders_->use();
        GLuint nb_lights = 8;
        GLint lights_id, nb_lights_id, light_color_id;
        std::vector<GLfloat> lights;

        lights_id = glGetUniformLocation(shaders_->getId(), "lights");
        nb_lights_id = glGetUniformLocation(shaders_->getId(), "nb_light");
        light_color_id = glGetUniformLocation(shaders_->getId(), "light_color");

        glUniform1i(nb_lights_id, nb_lights);
        
        Vector3 color{1.0f, 1.0f, 1.0f};
        glUniform3fv(light_color_id, 1, color.data());

        for (size_t i = 0; i < nb_lights; i++) {
            GLfloat z = -41.7f + i * (83.8f / nb_lights) + (83.f / (nb_lights * 2));

            lights.push_back(14.0f);
            lights.push_back(4.5f);
            lights.push_back(z);
        }

        glUniform3fv(lights_id, nb_lights, lights.data());
    }

    void Scene::init(Camera camera) {
        for (size_t i = 0; i < station_obj.size(); i++) {
            station_obj.at(i).get()->init_default(camera);
            station_obj.at(i).get()->init_lights();
        }

        for (size_t i = 0; i < station_signals.size(); i++) {
            station_signals.at(i).get()->init_default(camera);
        }
    }

    void load_mesh(const aiMesh* src, std::vector<GLfloat>& vec) {
        for (size_t i = 0; i < src->mNumVertices; i++) {
            vec.push_back(src->mVertices[i].x);
            vec.push_back(src->mVertices[i].y);
            vec.push_back(src->mVertices[i].z);

            vec.push_back(src->mNormals[i].x);
            vec.push_back(src->mNormals[i].y);
            vec.push_back(src->mNormals[i].z);

            vec.push_back(src->mTextureCoords[0][i].x);
            vec.push_back(src->mTextureCoords[0][i].y);
        }
    }

    void load_indices(const aiMesh* src, std::vector<GLuint>& vec) {
        for (size_t i = 0; i < src->mNumFaces; i++) {
            vec.push_back(src->mFaces[i].mIndices[0]);
            vec.push_back(src->mFaces[i].mIndices[1]);
            vec.push_back(src->mFaces[i].mIndices[2]);
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
        glVertexAttribPointer(vertex_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
        glVertexAttribPointer(vertex_uv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(2 * 3 * sizeof(GLfloat)));
        
        glEnableVertexAttribArray(vertex_position);
        glEnableVertexAttribArray(vertex_normal);
        glEnableVertexAttribArray(vertex_uv);

        glGenBuffers(1, &ebo_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

        object.state_ = true;
        object.nb_vertices_ = src->mNumVertices;
        object.nb_indices_ = indices.size();
        return object;
    }

    void GLobject::draw(Vector3 color) {
        shaders_->use();
        GLint color_id = glGetUniformLocation(shaders_->getId(), "color");

        glUniform3fv(color_id, 1, color.data());

        glBindVertexArray(vao_id_);

        glDrawElements(GL_TRIANGLES, nb_indices_, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, nb_vertices_);

        glBindVertexArray(0);
    }

    void GLobjectTex::draw(Vector3 color) {
        shaders_->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_->id());

        GLuint texture_id = glGetUniformLocation(shaders_->getId(), "signal_texture");
        glUniform1i(texture_id, 0);

        GLobject::draw(color);
    }

    void Scene::draw() {
        glClearColor(0.0f, 1.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quai1->draw(Vector3{0.4f, 0.4f, 0.4});
        quai2->draw(Vector3{0.4f, 0.4f, 0.4f});
        voute->draw(Vector3{1.0f, 1.0f, 1.0f});
        voies->draw(Vector3{0.1f, 0.1f, 0.1f});
        sous_quai1->draw(Vector3{0.1f, 0.1f, 0.1f});
        sous_quai2->draw(Vector3{0.1f, 0.1f, 0.1f});
        mur1->draw(Vector3{1.0f, 1.0f, 1.0f});
        mur2->draw(Vector3{1.0f, 1.0f, 1.0f});

        for (size_t i = 0; i < station_signals.size(); i++)
            station_signals.at(i).get()->draw({1.0f, 1.0f, 1.0f});
    }


    void Scene::import(const std::string& src) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate);

        if (!scene) { std::cout << "ECHEC DE LA SCENE" << std::endl; return; }

        // shaders compilation
        auto quai_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_quai.shd"));
        auto voute_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_voute.shd"));
        auto voies_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_voies.shd"));
        auto texture_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_texture.shd"));
        auto signal_texture = std::make_shared<Texture>("assets/textures/reaumur.png");

        std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

        for (size_t i = 0; i < scene->mNumMeshes; i++) {
            std::cout << "Mesh n°" << i << ": " << scene->mMeshes[i]->mName.C_Str() << '\n';
            std::string name = scene-> mMeshes[i]->mName.C_Str();

            if (name == "quai.1") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], quai_shd));

                station_obj.push_back(std::move(obj));
                quai1 = station_obj.back().get();
            }
            else if (name == "quai.2") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], quai_shd));

                station_obj.push_back(std::move(obj));
                quai2 = station_obj.back().get();
            }
            else if (name == "voies") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voies_shd));

                station_obj.push_back(std::move(obj));
                voies = station_obj.back().get();
            }
            else if (name == "voute") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voute_shd));

                station_obj.push_back(std::move(obj));
                voute = station_obj.back().get();
            }
            else if (name == "mur.1") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voute_shd));

                station_obj.push_back(std::move(obj));
                mur1 = station_obj.back().get();
            }
            else if (name == "mur.2") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voute_shd));

                station_obj.push_back(std::move(obj));
                mur2 = station_obj.back().get();
            }
            else if (name == "sous.quai.1") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voies_shd));

                station_obj.push_back(std::move(obj));
                sous_quai1 = station_obj.back().get();
            }
            else if (name == "sous.quai.2") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voies_shd));

                station_obj.push_back(std::move(obj));
                sous_quai2 = station_obj.back().get();
            }
            else if (name.find("panneau") != std::string::npos) {
                auto obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], texture_shd), signal_texture);

                station_signals.push_back(std::move(obj));
            }
        }

    }
}
