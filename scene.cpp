#include "scene.hh"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace gl {
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
                doors.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, offset);

        for (size_t i = 0; i < doors.size(); i++)
            if (!doors.at(i).get()->transparent() && doors.at(i).get()->overlay())
                doors.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, offset);

        for (size_t i = 0; i < doors.size(); i++)
            if (doors.at(i).get()->transparent() && !doors.at(i).get()->overlay())
                doors.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, offset);

        for (size_t i = 0; i < doors.size(); i++)
            if (doors.at(i).get()->transparent() && doors.at(i).get()->overlay())
                doors.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, offset);
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
                body_.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, z_position_});

        for (size_t i = 0; i < body_.size(); i++)
            if (!body_.at(i).get()->transparent() && body_.at(i).get()->overlay())
                body_.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, z_position_});

        GLfloat deport_prop = clamp01(door_opening_ / 0.25f);
        GLfloat slide_prop = clamp01((door_opening_ - 0.25f) / 0.75f);
        GLfloat deport_offset = -0.08f * deport_prop;
        GLfloat slide_offset = 0.8f * slide_prop;

        Vector3 left_offset{deport_offset, 0.0f, -slide_offset + z_position_};
        Vector3 right_offset{deport_offset, 0.0f, slide_offset + z_position_};

        for (size_t i = 0; i < textures_.size(); i++)
            textures_.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, z_position_});

        for (size_t i = 0; i < body_.size(); i++)
            if (body_.at(i).get()->transparent())
                body_.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, z_position_});

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
            station_signals.at(i).get()->draw();
        for (size_t i = 0; i < station_plans.size(); i++)
            station_plans.at(i).get()->draw();

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

    void Metro::run() {
        z_position_start_ = z_position_;

        if (z_position_ == -100.0f) z_position_target_ = 0.0f;
        else if (z_position_ == 0.0f) z_position_target_ = 100.0f;
        else z_position_target_ = -100.0f;
    }

    bool Metro::update_metro() {
        // DOORS
        const GLfloat door_step = 0.012f;

        if (door_opening_ < target_door_opening_) {
            door_opening_ = std::min(target_door_opening_, door_opening_ + door_step);
            return true;
        }

        if (door_opening_ > target_door_opening_) {
            door_opening_ = std::max(target_door_opening_, door_opening_ - door_step);
            return true;
        }

        // TRAIN
        GLfloat train_distance;
        if (z_position_start_ == 0.0f)
            train_distance = z_position_ - z_position_start_;
        else
            train_distance = z_position_target_ - z_position_;

        const GLfloat train_step = std::max(0.0001f, 0.045f * std::log1p(std::abs(train_distance)));

        if (z_position_ < z_position_target_) {
            z_position_ = std::min(z_position_target_, z_position_ + train_step);
            return true;
        }

        if (z_position_ > z_position_target_) {
            z_position_ = std::max(z_position_target_, z_position_ - train_step);
            return true;
        }

        return false;
    }

    void Scene::toggle_doors() {
        if (metro)
            metro->toggle_doors();
    }
    
    void Scene::run_metro() {
        if (metro)
            metro->run();
    }

    bool Scene::update_metro() {
        return metro && metro->update_metro();
    }
}
