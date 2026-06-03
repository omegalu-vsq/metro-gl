#include "scene.hh"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace gl {
    void GLobject::init(Camera camera) {
        shaders_.use();
        shaders_.init_POV(camera);

        GLuint nb_lights = 8;
        GLint lights_id, nb_lights_id, light_color_id;
        std::vector<GLfloat> lights;

        lights_id = glGetUniformLocation(shaders_.getId(), "lights");
        nb_lights_id = glGetUniformLocation(shaders_.getId(), "nb_light");
        light_color_id = glGetUniformLocation(shaders_.getId(), "light_color");

        glUniform1i(nb_lights_id, nb_lights);
        
        Vector3 color{1.0f, 1.0f, 1.0f};
        glUniform3fv(light_color_id, 1, color.data());

        for (size_t i = 0; i < nb_lights; i++) {
            GLfloat z = -41.7f + i * (83.8f / nb_lights) + (83.f / (nb_lights * 2));

            lights.push_back(14.0f);
            lights.push_back(8.5f);
            lights.push_back(z);
        }

        glUniform3fv(lights_id, nb_lights, lights.data());
    }

    void Scene::init_POV(Camera camera) {
        quai1.get()->init(camera);
        quai2.get()->init(camera);
        voute.get()->init(camera);
        voies.get()->init(camera);
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

    GLobject GLobject::make_object(const aiMesh* src, ShaderProgram&& shaders) {
        GLobject object{std::move(shaders)};

        std::vector<GLfloat> buffer;
        load_mesh(src, buffer);

        std::vector<GLuint> indices;
        load_indices(src, indices);

        GLuint vbo_id;
        GLuint ebo_id;
        GLint vertex_position = glGetAttribLocation(object.shaders_.getId(), "position");
        GLint vertex_normal = glGetAttribLocation(object.shaders_.getId(), "normal");
        GLint vertex_uv = glGetAttribLocation(object.shaders_.getId(), "uv");

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
        shaders_.use();
        GLint color_id = glGetUniformLocation(shaders_.getId(), "color");

        glUniform3fv(color_id, 1, color.data());

        glBindVertexArray(vao_id_);

        glDrawElements(GL_TRIANGLES, nb_indices_, GL_UNSIGNED_INT, 0);
        //glDrawArrays(GL_TRIANGLES, 0, nb_vertices_);

        glBindVertexArray(0);
    }

    void Scene::draw() {
        glClearColor(0.0f, 1.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quai1.get()->draw(Vector3{0.8f, 0.2f, 0.3f});
        quai2.get()->draw(Vector3{0.1f, 0.8f, 0.3f});
        voute.get()->draw(Vector3{0.1f, 0.2f, 0.8f});
        voies.get()->draw(Vector3{0.7f, 0.8f, 0.3f});
    }


    void Scene::import(const std::string& src) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate);

        if (!scene) { std::cout << "ECHEC DE LA SCENE" << std::endl; return; }

        std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

        for (size_t i = 0; i < scene->mNumMeshes; i++) {
            std::cout << "Mesh n°" << i << ": " << scene->mMeshes[i]->mName.C_Str() << '\n';
            std::string name = scene->mMeshes[i]->mName.C_Str();

            if (name == "quai.1") {
                auto shaders = ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_quai.shd");

                GLobject obj = GLobject::make_object(scene->mMeshes[i], std::move(shaders));

                quai1 = std::make_unique<GLobject>(std::move(obj));
            }
            if (name == "quai.2") {
                auto shaders = ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_quai.shd");

                GLobject obj = GLobject::make_object(scene->mMeshes[i], std::move(shaders));

                quai2 = std::make_unique<GLobject>(std::move(obj));
            }
            if (name == "voies") {
                auto shaders = ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_voies.shd");

                GLobject obj = GLobject::make_object(scene->mMeshes[i], std::move(shaders));

                voies = std::make_unique<GLobject>(std::move(obj));
            }
            if (name == "voute") {
                auto shaders = ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_voute.shd");

                GLobject obj = GLobject::make_object(scene->mMeshes[i], std::move(shaders));

                voute = std::make_unique<GLobject>(std::move(obj));
            }
        }

    }
}
