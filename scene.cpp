#include "scene.hpp"

#include "LTC.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <GL/freeglut.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace gl {
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

    void Metro::init(Camera camera, const std::vector<std::unique_ptr<GLlight>>& lights, bool update_lights) {
        for (size_t i = 0; i < body_.size(); i++) {
            body_.at(i).get()->init_default(camera);
            if (update_lights)
                body_.at(i).get()->init_lights(lights);
        }

        for (size_t i = 0; i < textures_.size(); i++) {
            textures_.at(i).get()->init_default(camera);
            if (update_lights)
                textures_.at(i).get()->init_lights(lights);
        }

        for (size_t i = 0; i < left_doors_.size(); i++) {
            left_doors_.at(i).get()->init_default(camera);
            if (update_lights)
                left_doors_.at(i).get()->init_lights(lights);
        }

        for (size_t i = 0; i < right_doors_.size(); i++) {
            right_doors_.at(i).get()->init_default(camera);
            if (update_lights)
                right_doors_.at(i).get()->init_lights(lights);
        }

        if (arcs_)
            arcs_.get()->init_default(camera);
    }

    void Metro::init_metro_lights() {
        for (size_t i = 0; i < body_.size(); i++)
            body_.at(i).get()->init_metro_lights(z_position_, 1.0f);

        for (size_t i = 0; i < textures_.size(); i++)
            textures_.at(i).get()->init_metro_lights(z_position_, 0.3f);

        for (size_t i = 0; i < left_doors_.size(); i++)
            left_doors_.at(i).get()->init_metro_lights(z_position_, 0.12f);

        for (size_t i = 0; i < right_doors_.size(); i++)
            right_doors_.at(i).get()->init_metro_lights(z_position_, 0.12f);
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

        draw_doors(left_doors_, left_offset);
        draw_doors(right_doors_, right_offset);

        for (size_t i = 0; i < body_.size(); i++)
            if (body_.at(i).get()->transparent())
                body_.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f}, Vector3{0.0f, 0.0f, z_position_});

        if (arcs_ && arc_time_ >= 0.0f && z_position_ != z_position_target_) {
            arc_shd_.get()->use();
            GLuint id = arc_shd_.get()->getId();

            glUniform1f(glGetUniformLocation(id, "anim_time"), arc_time_);
            glUniform1f(glGetUniformLocation(id, "arc_seed"), arc_seed_);
            glUniform1i(glGetUniformLocation(id, "active_arc"), active_arc_);

            glLineWidth(4.0f);
            arcs_.get()->draw(Vector3{0.7f, 0.85f, 1.0f}, Vector3{0.0f, 0.0f, z_position_});
            glLineWidth(1.0f);
        }
    }

    void Metro::draw_depth(ShaderProgram& shader) {
        Vector3 offset{0.0f, 0.0f, z_position_};

        for (size_t i = 0; i < body_.size(); i++)
            if (!body_.at(i)->transparent())
                body_.at(i)->draw_depth(shader, offset);

        for (size_t i = 0; i < textures_.size(); i++)
            textures_.at(i)->draw_depth(shader, offset);

        GLfloat deport_offset = -0.08f * clamp01(door_opening_ / 0.25f);
        GLfloat slide_offset = 0.8f * clamp01((door_opening_ - 0.25f) / 0.75f);

        Vector3 left_offset{deport_offset, 0.0f, -slide_offset + z_position_};
        Vector3 right_offset{deport_offset, 0.0f, slide_offset + z_position_};

        for (size_t i = 0; i < left_doors_.size(); i++)
            if (!left_doors_.at(i)->transparent()) left_doors_.at(i)->draw_depth(shader, left_offset);

        for (size_t i = 0; i < right_doors_.size(); i++)
            if (!right_doors_.at(i)->transparent()) right_doors_.at(i)->draw_depth(shader, right_offset);
    }

    void Metro::import(const std::string& body, const std::string& left_doors, const std::string& right_doors) {
        import_body(body);
        import_doors(left_doors, true);
        import_doors(right_doors, false);
        import_arcs();
    }

    void Metro::import_arcs() {
        arc_shd_ = std::make_shared<ShaderProgram>(ShaderProgram::make_program(
            "shaders/arc.vert", "shaders/arc.frag", "", "shaders/arc.tesc", "shaders/arc.tese"));

        if (!arc_shd_.get()->is_ready()) return;

        std::vector<Vector3> points;
        const GLfloat bogies[] = {-32.0f, -23.0f, -17.0f, -8.0f, -2.0f, 7.0f, 13.0f, 22.0f, 28.0f, 37.0f};

        for (GLfloat z : bogies) {
            z = 37.0f + (z - 37.0f) * 0.995f - 1.25f;
            points.push_back(Vector3{13.575f, -0.40f, z - 0.325f});
            points.push_back(Vector3{13.575f, -0.42f, z + 0.325f});
        }

        arc_count_ = points.size() / 2;
        arcs_ = std::make_unique<GLobject>(GLobject::make_arcs(arc_shd_, points));

        std::srand(static_cast<unsigned int>(std::time(nullptr)));
    }

    void Metro::update_arcs() {
        if (!arcs_) return;

        const GLint arc_duration = 200;
        GLint now = glutGet(GLUT_ELAPSED_TIME);

        if (z_position_ == z_position_target_) {
            arc_time_ = -1.0f;
            next_arc_time_ = now + 250;
            return;
        }

        if (arc_time_ >= 0.0f) {
            arc_time_ = static_cast<GLfloat>(now - arc_start_time_) / arc_duration;
            if (arc_time_ >= 1.0f) arc_time_ = -1.0f;
        }

        if (arc_time_ < 0.0f && now >= next_arc_time_) {
            active_arc_ = std::rand() % arc_count_;
            arc_seed_ = std::rand() % 10000;
            arc_start_time_ = now;
            arc_time_ = 0.0f;
            next_arc_time_ = now + arc_duration + 250 + std::rand() % 1000;
        }
    }

    void Metro::import_body(const std::string& src) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

        if (!scene) { std::cout << "/!\\ Metro::import() failed: " << importer.GetErrorString() << std::endl; return; }

        auto metro_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/metro.frag"));
        auto metro_texture_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/metro_texture.frag"));
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
                auto obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], metro_texture_shd), ratp_texture);
                obj->set_roughness(0.05f);

                textures_.push_back(std::move(obj));
            }
            else {
                auto obj = std::make_unique<GLobjectMetro>(GLobject::make_object(scene->mMeshes[i], metro_shd), color, alpha, door, overlay);
                obj->set_roughness(0.05f);

                body_.push_back(std::move(obj));
            }
        }
    }

    void Metro::import_doors(const std::string& src, bool left) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

        if (!scene) { std::cout << "/!\\ Metro::import_doors() failed: " << importer.GetErrorString() << std::endl; return; }

        auto metro_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/metro.frag"));

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
            obj->set_roughness(0.05f);

            if (left)
                left_doors_.push_back(std::move(obj));
            else
                right_doors_.push_back(std::move(obj));
        }
    }

    void Metro::toggle_doors() {
        target_door_opening_ = target_door_opening_ > 0.5f ? 0.0f : 1.0f;
    }

    void Metro::run() {
        z_position_start_ = z_position_;

        if (z_position_ == -150.0f) z_position_target_ = 0.0f;
        else if (z_position_ == 0.0f) z_position_target_ = 150.0f;
        else z_position_target_ = -150.0f;
    }

    void Metro::reset_entry() {
        arc_time_ = -1.0f;
        door_opening_ = 0.0f;
        target_door_opening_ = 0.0f;
        z_position_ = -150.0f;
        z_position_start_ = -150.0f;
        z_position_target_ = -150.0f;
    }

    bool Metro::update_metro() {
        update_arcs();

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

    void Scene::import(const std::string& src) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(src, aiProcess_Triangulate);

        if (!scene) { std::cout << "/!\\ Scene::import() failed: " << importer.GetErrorString() << std::endl; return; }

        auto quai_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/quai.frag"));
        auto voute_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/voute.frag"));
        auto mur_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/mur.frag"));
        auto ballast_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/ballast.vert", "shaders/voies.frag", "", "shaders/ballast.tesc", "shaders/ballast.tese"));
        auto color_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/color.frag"));
        auto rail_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/rail.frag"));
        auto tunnel_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/tunnel.frag"));
        auto emaille_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/emaille.frag"));
        auto plan_shd = std::make_shared<ShaderProgram>(ShaderProgram::make_program("shaders/default.vert", "shaders/plan.frag"));

        shadow_receivers_ = {quai_shd, voute_shd, rail_shd, tunnel_shd, ballast_shd};

        auto signal_texture = std::make_shared<Texture>("assets/textures/reaumur.png");
        auto plan_metro_texture = std::make_shared<Texture>("assets/textures/plan-metro.png", true, true);
        auto plan_idf_texture = std::make_shared<Texture>("assets/textures/plan-idf.png", true, true);
        auto plan_bus_texture = std::make_shared<Texture>("assets/textures/plan-bus.png", true, true);
        auto plan_quartier_texture = std::make_shared<Texture>("assets/textures/plan-quartier.png", true, true);
        auto ballast_texture = std::make_shared<Texture>("assets/textures/ballast.jpg");
        std::vector<std::shared_ptr<Texture>> panam_textures = {
            std::make_shared<Texture>("assets/textures/panam1.png", true, true),
            std::make_shared<Texture>("assets/textures/panam2.png", true, true)
        };
        std::vector<std::shared_ptr<Texture>> pub_textures = {
            std::make_shared<Texture>("assets/textures/pub1.png", true, true),
            std::make_shared<Texture>("assets/textures/pub2.png", true, true),
            std::make_shared<Texture>("assets/textures/pub3.png", true, true),
            std::make_shared<Texture>("assets/textures/pub4.png", true, true),
            std::make_shared<Texture>("assets/textures/pub5.png", true, true),
            std::make_shared<Texture>("assets/textures/pub6.png", true, true)
        };

        std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

        for (size_t i = 0; i < scene->mNumMeshes; i++) {
            std::cout << "Mesh n°" << i << ": " << scene->mMeshes[i]->mName.C_Str() << '\n';
            std::string name = scene->mMeshes[i]->mName.C_Str();

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
                auto obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], ballast_shd, 4), ballast_texture);
                obj->set_roughness(1.0f);

                station_obj.push_back(std::move(obj));
                voies = station_obj.back().get();
            }
            else if (name == "voute") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], voute_shd));
                obj->set_roughness(0.25f);

                station_obj.push_back(std::move(obj));
                voute = station_obj.back().get();
            }
            else if (name == "mur.1") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], mur_shd));
                obj->set_roughness(0.25f);

                station_obj.push_back(std::move(obj));
                mur1 = station_obj.back().get();
            }
            else if (name == "mur.2") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], mur_shd));
                obj->set_roughness(0.25f);

                station_obj.push_back(std::move(obj));
                mur2 = station_obj.back().get();
            }
            else if (name == "sous.quai.1") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], color_shd));

                station_obj.push_back(std::move(obj));
                sous_quai1 = station_obj.back().get();
            }
            else if (name == "sous.quai.2") {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], color_shd));

                station_obj.push_back(std::move(obj));
                sous_quai2 = station_obj.back().get();
            }
            else if (name.find("rail") != std::string::npos) {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], rail_shd));
                obj->set_roughness(0.2f);
                obj->set_metallic(1.0f);

                station_obj.push_back(std::move(obj));
                rails.push_back(station_obj.back().get());
            }
            else if (name.find("traverse") != std::string::npos) {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], quai_shd));
                obj->set_roughness(0.9f);

                station_obj.push_back(std::move(obj));
                traverses.push_back(station_obj.back().get());
            }
            else if (name.find("tige.") != std::string::npos) {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], color_shd));

                station_obj.push_back(std::move(obj));
                tiges.push_back(station_obj.back().get());
            }
            else if (name.find("tunnel") != std::string::npos) {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], tunnel_shd));
                obj->set_roughness(0.85f);

                station_obj.push_back(std::move(obj));
                tunnels.push_back(station_obj.back().get());
            }
            else if (name.find("bouchon") != std::string::npos) {
                auto obj = std::make_unique<GLobject>(GLobject::make_object(scene->mMeshes[i], color_shd));

                station_obj.push_back(std::move(obj));
                bouchons.push_back(station_obj.back().get());
            }
            else if (name.find("podotact") != std::string::npos) {
                auto obj = std::make_unique<GLobjectPodo>(GLobject::make_object(scene->mMeshes[i], quai_shd));

                podotactile_strips.push_back(std::move(obj));
            }
            else if (name.find("panneau") != std::string::npos) {
                auto obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], emaille_shd), signal_texture);
                obj->set_roughness(0.025f);

                station_signals.push_back(std::move(obj));
            }
            else if (name.find("plan") != std::string::npos) {
                std::unique_ptr<GLobjectTex> obj;
                if (name.find("metro") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], plan_shd), plan_metro_texture);
                else if (name.find("idf") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], plan_shd), plan_idf_texture);
                else if (name.find("bus") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], plan_shd), plan_bus_texture);
                else if (name.find("quartier") != std::string::npos)
                    obj = std::make_unique<GLobjectTex>(GLobject::make_object(scene->mMeshes[i], plan_shd), plan_quartier_texture);

                if (obj) {
                    obj->set_roughness(0.8f);
                    station_plans.push_back(std::move(obj));
                }
            }
            else if (name.find("pub.") != std::string::npos) {
                char pub_id = name.back();
                if ('1' <= pub_id && pub_id <= '6') {
                    size_t texture_id = pub_id - '1';
                    if (name.find("pub.2") != std::string::npos)
                        texture_id = 5 - texture_id;

                    auto obj = std::make_unique<GLobjectTex>(
                        GLobject::make_object(scene->mMeshes[i], plan_shd),
                        pub_textures.at(texture_id)
                    );
                    obj->set_roughness(0.8f);

                    station_plans.push_back(std::move(obj));
                }
            }
            else if (name.find("panam.") != std::string::npos) {
                char panam_id = name.back();
                if ('1' <= panam_id && panam_id <= '2') {
                    auto obj = std::make_unique<GLobjectTex>(
                        GLobject::make_object(scene->mMeshes[i], emaille_shd),
                        panam_textures.at(panam_id - '1')
                    );
                    obj->set_roughness(0.025f);

                    station_plans.push_back(std::move(obj));
                }
            }
        }

        // light tube fabrication
        const float tube_intensity = 100.0f;
        for (size_t i = 0; i < 8; i++) {
            GLfloat z = -38.7f + i * (83.8f / 8) + (83.f / 16);
            auto tube = GLlightTube::make_light_tube(Vector3{1.0f, 1.0f, 0.72f}, Vector3{19.0f, 4.5f, z}, tube_intensity, 0.08f, 2.0f, color_shd);
            station_lights.push_back(std::make_unique<GLlightTube>(tube.first));
            station_obj.push_back(std::move(tube.second));
        }

        for (size_t i = 0; i < 8; i++) {
            GLfloat z = -38.7f + i * (83.8f / 8) + (83.f / 16);
            auto tube = GLlightTube::make_light_tube(Vector3{1.0f, 1.0f, 0.72f}, Vector3{9.0f, 4.5f, z}, tube_intensity, 0.08f, 2.0f, color_shd);
            station_lights.push_back(std::make_unique<GLlightTube>(tube.first));
            station_obj.push_back(std::move(tube.second));
        }

        const float strip_width = 0.62f;
        const float white_width = strip_width;
        const float z_min = -36.754f;
        const float z_max = 42.115f;
        const float y_top = 0.573f;

        white_edge_strips.push_back(std::make_unique<GLobject>(
            GLobject::make_strip(quai_shd, 11.056f - white_width, 11.056f, y_top + 0.001f, z_min, z_max)));
        white_edge_strips.back()->set_roughness(0.9f);
        white_edge_strips.push_back(std::make_unique<GLobject>(
            GLobject::make_strip(quai_shd, 17.448f, 17.448f + white_width, y_top + 0.001f, z_min, z_max)));
        white_edge_strips.back()->set_roughness(0.9f);

        if (podotactile_strips.empty()) {
            podotactile_strips.push_back(std::make_unique<GLobjectPodo>(
                GLobject::make_strip(quai_shd, 11.056f - white_width - strip_width, 11.056f - white_width, y_top + 0.002f, z_min, z_max)));
            podotactile_strips.push_back(std::make_unique<GLobjectPodo>(
                GLobject::make_strip(quai_shd, 17.448f + white_width, 17.448f + white_width + strip_width, y_top + 0.002f, z_min, z_max)));
        }
    }

    void Scene::import_metro(const std::string& body, const std::string& left_doors, const std::string& right_doors) {
        metro = std::make_unique<Metro>();
        metro->import(body, left_doors, right_doors);
    }

    void Scene::import_ltc() {
        this->ltc1 = std::make_unique<Texture>(LTC1);
        this->ltc2 = std::make_unique<Texture>(LTC2);

        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, ltc1.get()->id());
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, ltc2.get()->id());
    }

    void Scene::import_shadow_controller() {
        if (station_lights.empty() || shadow_receivers_.empty()) return;

        std::vector<Vector3> positions;

        for (size_t i = 0; i < station_lights.size(); i++)
            positions.push_back(station_lights[i]->position());

        shadow_controller_ = std::make_unique<ShadowController>(positions);

        if (!shadow_controller_->ready()) {
            shadow_controller_.reset();
            return;
        }

        for (size_t i = 0; i < shadow_receivers_.size(); i++)
            shadow_controller_->apply(*shadow_receivers_[i], shadow_enabled_);

        shadow_dirty_ = true;
    }

    void Scene::toggle_shadow_controller() {
        if (!shadow_controller_) return;

        shadow_enabled_ = !shadow_enabled_;

        for (size_t i = 0; i < shadow_receivers_.size(); i++)
            shadow_controller_->apply(*shadow_receivers_[i], shadow_enabled_);

        shadow_dirty_ = true;
    }

    void Scene::toggle_shadow_debug() {
        shadow_debug_ = !shadow_debug_;

        for (size_t i = 0; i < shadow_receivers_.size(); i++) {
            shadow_receivers_.at(i)->use();
            glUniform1i(glGetUniformLocation(shadow_receivers_.at(i)->getId(), "shadow_debug"), shadow_debug_);
        }
    }

    void Scene::init(Camera camera, bool update_lights) {
        for (size_t i = 0; i < station_obj.size(); i++) {
            station_obj.at(i).get()->init_default(camera);
            if (update_lights)
                station_obj.at(i).get()->init_lights(station_lights);
        }

        for (size_t i = 0; i < station_signals.size(); i++) {
            station_signals.at(i).get()->init_default(camera);
            if (update_lights)
                station_signals.at(i).get()->init_lights(station_lights);
        }

        for (size_t i = 0; i < station_plans.size(); i++) {
            station_plans.at(i).get()->init_default(camera);
            if (update_lights)
                station_plans.at(i).get()->init_lights(station_lights);
        }

        for (size_t i = 0; i < white_edge_strips.size(); i++) {
            white_edge_strips.at(i).get()->init_default(camera);
            if (update_lights)
                white_edge_strips.at(i).get()->init_lights(station_lights);
        }

        for (size_t i = 0; i < podotactile_strips.size(); i++) {
            podotactile_strips.at(i).get()->init_default(camera);
            if (update_lights)
                podotactile_strips.at(i).get()->init_lights(station_lights);
        }

        if (metro)
            metro->init(camera, station_lights, update_lights);
    }

    void Scene::init_metro_lights() {
        if (!metro) return;

        GLfloat train_z = metro->z_position();

        for (size_t i = 0; i < station_obj.size(); i++)
            station_obj.at(i).get()->init_metro_lights(train_z, 1.0f);

        for (size_t i = 0; i < station_signals.size(); i++)
            station_signals.at(i).get()->init_metro_lights(train_z, 3.0f);

        for (size_t i = 0; i < station_plans.size(); i++)
            station_plans.at(i).get()->init_metro_lights(train_z, 0.5f);

        for (size_t i = 0; i < white_edge_strips.size(); i++)
            white_edge_strips.at(i).get()->init_metro_lights(train_z, 1.0f);

        for (size_t i = 0; i < podotactile_strips.size(); i++)
            podotactile_strips.at(i).get()->init_metro_lights(train_z, 1.0f);

        metro->init_metro_lights();
    }

    void Scene::draw() {
        if (shadow_controller_ && shadow_enabled_ && shadow_dirty_) {
            shadow_controller_->begin();

            for (size_t light = 0; light < shadow_controller_->light_count(); light++) {
                shadow_controller_->select_light(light);

                if (metro) metro->draw_depth(shadow_controller_->shader());

                mur1->draw_depth(shadow_controller_->shader());
                mur2->draw_depth(shadow_controller_->shader());

                for (size_t i = 0; i < tunnels.size(); i++)
                    tunnels.at(i)->draw_depth(shadow_controller_->shader());
            }

            shadow_controller_->end();
            shadow_dirty_ = false;
        }

        const GLfloat background[] = {0.0f, 1.0f, 0.1f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, background);
        glClear(GL_DEPTH_BUFFER_BIT);

        init_metro_lights();

        quai1->draw(Vector3{0.06f, 0.05f, 0.05f});
        quai2->draw(Vector3{0.06f, 0.05f, 0.05f});

        for (size_t i = 0; i < white_edge_strips.size(); i++)
            white_edge_strips.at(i).get()->draw(Vector3{0.99f, 0.98f, 0.95f});

        for (size_t i = 0; i < podotactile_strips.size(); i++)
            podotactile_strips.at(i).get()->draw(Vector3{1.0f, 1.0f, 1.0f});

        voute->draw(Vector3{1.0f, 1.0f, 1.0f});
        voies->draw(Vector3{1.0f, 1.0f, 1.0f});

        for (size_t i = 0; i < rails.size(); i++)
            rails.at(i)->draw(Vector3{1.0f, 1.0f, 1.0f});
        for (size_t i = 0; i < traverses.size(); i++)
            traverses.at(i)->draw(Vector3{0.04f, 0.04f, 0.04f});
        for (size_t i = 0; i < tiges.size(); i++)
            tiges.at(i)->draw(Vector3{0.08f, 0.08f, 0.08f});

        sous_quai1->draw(Vector3{0.1f, 0.1f, 0.1f});
        sous_quai2->draw(Vector3{0.1f, 0.1f, 0.1f});
        mur1->draw(Vector3{1.0f, 1.0f, 1.0f});
        mur2->draw(Vector3{1.0f, 1.0f, 1.0f});

        for (size_t i = 0; i < tunnels.size(); i++)
            tunnels.at(i)->draw(Vector3{0.05f, 0.05f, 0.05f});
        for (size_t i = 0; i < bouchons.size(); i++)
            bouchons.at(i)->draw(Vector3{0.0f, 0.0f, 0.0f});

        for (size_t i = 0; i < station_signals.size(); i++)
            station_signals.at(i).get()->draw();
        for (size_t i = 0; i < station_plans.size(); i++)
            station_plans.at(i).get()->draw();

        for (size_t i = 0; i < station_lights.size(); i++)
            station_lights.at(i).get()->draw();

        if (metro)
            metro->draw();
    }

    void Scene::toggle_doors() {
        if (metro)
            metro->toggle_doors();
    }

    void Scene::run_metro() {
        if (metro)
            metro->run();
    }

    void Scene::reset_metro_entry() {
        if (metro)
            metro->reset_entry();
        shadow_dirty_ = true;
    }

    bool Scene::update_metro() {
        bool moved = metro && metro->update_metro();

        if (moved) shadow_dirty_ = true;

        return moved;
    }

}
