#include "scene.hh"

#include <algorithm>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
            std::cerr << "Texture load failure: " << src << std::endl;
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

    Texture::~Texture() {
        if (id_) glDeleteTextures(1, &id_);
    }

    void GLobject::init_default(Camera camera) {
        shaders_->use();
        shaders_->init_POV(camera);
    }

    void GLobject::init_lights() {
        shaders_->use();
        GLuint nb_lights = 16;
        GLint lights_id, nb_lights_id, light_color_id;
        std::vector<GLfloat> lights;

        lights_id = glGetUniformLocation(shaders_->getId(), "lights");
        nb_lights_id = glGetUniformLocation(shaders_->getId(), "nb_light");
        light_color_id = glGetUniformLocation(shaders_->getId(), "light_color");

        glUniform1i(nb_lights_id, nb_lights);
        
        const GLfloat light_intensity = 0.55f;
        Vector3 color{light_intensity, light_intensity, light_intensity};
        glUniform3fv(light_color_id, 1, color.data());

        for (size_t i = 0; i < nb_lights / 2; i++) {
            GLfloat z = -41.7f + i * (83.8f / (nb_lights / 2)) + (83.f / (nb_lights));

            lights.push_back(19.0f);
            lights.push_back(4.0f);
            lights.push_back(z);
        }

        for (size_t i = 0; i < nb_lights / 2; i++) {
            GLfloat z = -41.7f + i * (83.8f / (nb_lights / 2)) + (83.f / (nb_lights));

            lights.push_back(9.0f);
            lights.push_back(4.0f);
            lights.push_back(z);
        }

        glUniform3fv(lights_id, nb_lights, lights.data());
    }

    void Scene::init(Camera camera) {
        for (size_t i = 0; i < station_obj.size(); i++) {
            station_obj.at(i).get()->init_default(camera);
            station_obj.at(i).get()->init_lights();
        }

        for (size_t i = 0; i < station_signals.size(); i++)
            station_signals.at(i).get()->init_default(camera);

        for (size_t i = 0; i < station_plans.size(); i++)
            station_plans.at(i).get()->init_default(camera);

        if (metro)
            metro->init(camera);
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

    void GLobject::draw(Vector3 color) {
        if (!state_) return;

        shaders_->use();
        GLint color_id = glGetUniformLocation(shaders_->getId(), "color");
        GLint offset_id = glGetUniformLocation(shaders_->getId(), "model_offset");

        glUniform3fv(color_id, 1, color.data());
        glUniform3f(offset_id, 0.0f, 0.0f, 0.0f);

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

    void GLobjectMetro::draw(Vector3) {
        draw_offset(Vector3{0.0f, 0.0f, 0.0f});
    }

    void GLobjectMetro::draw_offset(Vector3 offset) {
        if (!state_) return;

        shaders_->use();
        GLint color_id = glGetUniformLocation(shaders_->getId(), "color");
        GLint alpha_id = glGetUniformLocation(shaders_->getId(), "alpha");
        GLint offset_id = glGetUniformLocation(shaders_->getId(), "model_offset");

        glUniform3fv(color_id, 1, color_.data());
        glUniform1f(alpha_id, alpha_);
        glUniform3fv(offset_id, 1, offset.data());

        glBindVertexArray(vao_id_);
        glDrawElements(GL_TRIANGLES, nb_indices_, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void load_material_color(const aiScene* scene, const aiMesh* mesh, Vector3& color, GLfloat& alpha) {
        color = Vector3{1.0f, 1.0f, 1.0f};
        alpha = 1.0f;

        if (mesh->mMaterialIndex >= scene->mNumMaterials) return;

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        aiColor4D material_color;

        if (aiGetMaterialColor(material, AI_MATKEY_BASE_COLOR, &material_color) != AI_SUCCESS &&
            aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &material_color) != AI_SUCCESS) {
            return;
        }

        color = Vector3{material_color.r, material_color.g, material_color.b};
        alpha = material_color.a;

        float opacity = alpha;
        if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
            alpha = opacity;

        alpha = std::max(0.0f, std::min(1.0f, alpha));
    }

    bool is_metro_overlay(Vector3 color) {
        GLfloat min_color = std::min(color.x_, std::min(color.y_, color.z_));
        GLfloat max_color = std::max(color.x_, std::max(color.y_, color.z_));
        GLfloat average = (color.x_ + color.y_ + color.z_) / 3.0f;

        return max_color - min_color > 0.08f || average > 0.85f;
    }

    bool material_has_texture(const aiScene* scene, const aiMesh* mesh) {
        if (mesh->mMaterialIndex >= scene->mNumMaterials) return false;

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        return material->GetTextureCount(aiTextureType_DIFFUSE) > 0 ||
               material->GetTextureCount(aiTextureType_BASE_COLOR) > 0;
    }

    void draw_doors(std::vector<std::unique_ptr<GLobjectMetro>>& doors, Vector3 offset) {
        for (size_t i = 0; i < doors.size(); i++)
            if (!doors.at(i).get()->transparent() && !doors.at(i).get()->overlay())
                doors.at(i).get()->draw_offset(offset);

        for (size_t i = 0; i < doors.size(); i++)
            if (!doors.at(i).get()->transparent() && doors.at(i).get()->overlay())
                doors.at(i).get()->draw_offset(offset);

        for (size_t i = 0; i < doors.size(); i++)
            if (doors.at(i).get()->transparent() && !doors.at(i).get()->overlay())
                doors.at(i).get()->draw_offset(offset);

        for (size_t i = 0; i < doors.size(); i++)
            if (doors.at(i).get()->transparent() && doors.at(i).get()->overlay())
                doors.at(i).get()->draw_offset(offset);
    }

    GLfloat clamp01(GLfloat value) {
        return std::max(0.0f, std::min(1.0f, value));
    }

    void Metro::init(Camera camera) {
        for (size_t i = 0; i < body_.size(); i++) {
            body_.at(i).get()->init_default(camera);
            body_.at(i).get()->init_lights();
        }

        for (size_t i = 0; i < textures_.size(); i++) {
            textures_.at(i).get()->init_default(camera);
            textures_.at(i).get()->init_lights();
        }

        for (size_t i = 0; i < left_doors_.size(); i++) {
            left_doors_.at(i).get()->init_default(camera);
            left_doors_.at(i).get()->init_lights();
        }

        for (size_t i = 0; i < right_doors_.size(); i++) {
            right_doors_.at(i).get()->init_default(camera);
            right_doors_.at(i).get()->init_lights();
        }
    }

    void Metro::draw() {
        for (size_t i = 0; i < body_.size(); i++)
            if (!body_.at(i).get()->transparent() && !body_.at(i).get()->overlay())
                body_.at(i).get()->draw({1.0f, 1.0f, 1.0f});

        for (size_t i = 0; i < body_.size(); i++)
            if (!body_.at(i).get()->transparent() && body_.at(i).get()->overlay())
                body_.at(i).get()->draw({1.0f, 1.0f, 1.0f});

        GLfloat deport = clamp01(door_opening_ / 0.25f);
        GLfloat slide = clamp01((door_opening_ - 0.25f) / 0.75f);
        GLfloat deport_offset = -0.08f * deport;
        GLfloat slide_offset = 0.8f * slide;

        Vector3 left_offset{deport_offset, 0.0f, -slide_offset};
        Vector3 right_offset{deport_offset, 0.0f, slide_offset};

        for (size_t i = 0; i < textures_.size(); i++)
            textures_.at(i).get()->draw({1.0f, 1.0f, 1.0f});

        for (size_t i = 0; i < body_.size(); i++)
            if (body_.at(i).get()->transparent())
                body_.at(i).get()->draw({1.0f, 1.0f, 1.0f});

        draw_doors(left_doors_, left_offset);
        draw_doors(right_doors_, right_offset);
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
        for (size_t i = 0; i < station_plans.size(); i++)
            station_plans.at(i).get()->draw({1.0f, 1.0f, 1.0f});

        if (metro)
            metro->draw();
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

        // texture import 
        auto signal_texture = std::make_shared<Texture>("assets/textures/reaumur.png");
        auto plan_metro_texture = std::make_shared<Texture>("assets/textures/plan-metro.png", true, true);
        auto plan_idf_texture = std::make_shared<Texture>("assets/textures/plan-idf.png", true, true);
        auto plan_bus_texture = std::make_shared<Texture>("assets/textures/plan-bus.png", true, true);
        auto plan_quartier_texture = std::make_shared<Texture>("assets/textures/plan-quartier.png", true, true);

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
            else if (name.find("plan") != std::string::npos) {
                std::unique_ptr<GLobjectTex> obj;
                if (name.find("metro") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], texture_shd), plan_metro_texture);
                else if (name.find("idf") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], texture_shd), plan_idf_texture);
                else if (name.find("bus") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], texture_shd), plan_bus_texture);
                else if (name.find("quartier") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], texture_shd), plan_quartier_texture);
              
                if (obj)
                    station_plans.push_back(std::move(obj));
            }
        }

    }

    void Metro::import(const std::string& body, const std::string& left_doors, const std::string& right_doors) {
        import_body(body);
        import_doors(left_doors, true);
        import_doors(right_doors, false);
    }

    void Metro::import_body(const std::string& src) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

        if (!scene) { std::cout << "ECHEC DU METRO: " << importer.GetErrorString() << std::endl; return; }

        auto metro_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_metro.shd"));
        auto metro_texture_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_texture_light.shd"));
        auto ratp_texture = std::make_shared<Texture>("assets/textures/ratp.jpg", false, true);

        std::cout << "Metro meshes: " << scene->mNumMeshes << std::endl;

        for (size_t i = 0; i < scene->mNumMeshes; i++) {
            std::string name = scene->mMeshes[i]->mName.C_Str();
            Vector3 color;
            GLfloat alpha;

            load_material_color(scene, scene->mMeshes[i], color, alpha);

            bool door = name.find("door.") != std::string::npos;

            if (door)
                std::cout << "DOOR NAME: " << scene->mMeshes[i]->mName.C_Str();

            bool overlay = is_metro_overlay(color);

            if (material_has_texture(scene, scene->mMeshes[i])) {
                auto obj = std::make_unique<GLobjectTexLight>(GLobject::make_object(scene->mMeshes[i], metro_texture_shd), ratp_texture);

                textures_.push_back(std::move(obj));
            }
            else {
                auto obj = std::make_unique<GLobjectMetro>(GLobject::make_object(scene->mMeshes[i], metro_shd), color, alpha, door, overlay);

                body_.push_back(std::move(obj));
            }
        }
    }

    void Metro::import_doors(const std::string& src, bool left) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

        if (!scene) { std::cout << "ECHEC DES PORTES: " << importer.GetErrorString() << std::endl; return; }

        auto metro_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/vertex.shd", "shaders/frag_metro.shd"));

        for (size_t i = 0; i < scene->mNumMeshes; i++) {
            Vector3 color;
            GLfloat alpha;

            load_material_color(scene, scene->mMeshes[i], color, alpha);

            auto obj = std::make_unique<GLobjectMetro>(
                GLobject::make_object(scene->mMeshes[i], metro_shd),
                color,
                alpha,
                true,
                is_metro_overlay(color)
            );

            if (left)
                left_doors_.push_back(std::move(obj));
            else
                right_doors_.push_back(std::move(obj));
        }

        std::cout << "Metro doors: " << left_doors_.size() << " left, " << right_doors_.size() << " right" << std::endl;
    }

    void Scene::import_metro(const std::string& body, const std::string& left_doors, const std::string& right_doors) {
        metro = std::make_unique<Metro>();
        metro->import(body, left_doors, right_doors);
    }

    void Metro::toggle_doors() {
        target_door_opening_ = target_door_opening_ > 0.5f ? 0.0f : 1.0f;
    }

    bool Metro::update_doors() {
        const GLfloat step = 0.012f;

        if (door_opening_ < target_door_opening_) {
            door_opening_ = std::min(target_door_opening_, door_opening_ + step);
            return true;
        }

        if (door_opening_ > target_door_opening_) {
            door_opening_ = std::max(target_door_opening_, door_opening_ - step);
            return true;
        }

        return false;
    }

    void Scene::toggle_doors() {
        if (metro)
            metro->toggle_doors();
    }

    bool Scene::update_doors() {
        return metro && metro->update_doors();
    }
}
