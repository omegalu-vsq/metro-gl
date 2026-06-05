#pragma once

#include "program.hh"
#include "vector.hh"

#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <memory>
#include <utility>

namespace gl {
    class Texture {
        GLuint id_;

        public:
        Texture(const std::string& src, bool flip_y = false, bool flip_x = false);
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        ~Texture();

        GLuint id() const { return id_; }
    };

    class GLobject {
        protected:
        std::shared_ptr<ShaderProgram> shaders_;
        GLuint vao_id_;
        matrix4 transform_;
        GLuint nb_vertices_;
        GLuint nb_indices_;

        bool state_;

        public:
        GLobject(std::shared_ptr<ShaderProgram> shaders)
            : shaders_(std::move(shaders)), vao_id_(0), transform_(), nb_vertices_(0), nb_indices_(0), state_(false) {}
        GLobject(const GLobject&) = delete;
        GLobject(GLobject&&) = default;
        virtual ~GLobject() = default;

        void init_default(Camera camera);
        void init_lights();

        virtual void draw(Vector3 color);

        static GLobject make_object(const aiMesh* src, std::shared_ptr<ShaderProgram> shaders);
    };

    class GLobjectTex : public GLobject {
        std::shared_ptr<Texture> texture_;

        public:
        GLobjectTex(GLobject&& object, std::shared_ptr<Texture> texture)
            : GLobject(std::move(object)), texture_(std::move(texture)) {}

        void draw(Vector3 color) override;
    };

    class GLobjectTexLight : public GLobjectTex {
        public:
        GLobjectTexLight(GLobject&& object, std::shared_ptr<Texture> texture)
            : GLobjectTex(std::move(object), std::move(texture)) {}
    };

    class GLobjectMetro : public GLobject {
        Vector3 color_;
        GLfloat alpha_;
        bool door_;
        bool overlay_;

        public:
        GLobjectMetro(GLobject&& object, Vector3 color, GLfloat alpha, bool door, bool overlay)
            : GLobject(std::move(object)), color_(color), alpha_(alpha), door_(door), overlay_(overlay) {}

        void draw(Vector3 color) override;
        void draw_offset(Vector3 offset);
        bool transparent() const { return alpha_ < 0.99f; }
        bool overlay() const { return overlay_; }
        bool door() const { return door_; }
    };

    class Metro {
        std::vector<std::unique_ptr<GLobjectMetro>> body_;
        std::vector<std::unique_ptr<GLobjectTexLight>> textures_;
        std::vector<std::unique_ptr<GLobjectMetro>> left_doors_;
        std::vector<std::unique_ptr<GLobjectMetro>> right_doors_;

        GLfloat door_opening_ = 0.0f;
        GLfloat target_door_opening_ = 0.0f;

        void import_body(const std::string& src);
        void import_doors(const std::string& src, bool left);

        public:
        Metro() = default;

        void import(const std::string& body, const std::string& left_doors, const std::string& right_doors);
        void init(Camera camera);
        void draw();
        void toggle_doors();
        bool update_doors();
    };

    class Scene {
        public:

        std::vector<std::unique_ptr<GLobject>> station_obj;
        std::vector<std::unique_ptr<GLobjectTex>> station_signals;
        std::vector<std::unique_ptr<GLobjectTex>> station_plans;
        std::unique_ptr<Metro> metro;
        
        GLobject* quai1 = nullptr;
        GLobject* quai2 = nullptr;
        GLobject* sous_quai1 = nullptr;
        GLobject* sous_quai2 = nullptr;

        GLobject* voies = nullptr;

        GLobject* voute = nullptr;
        GLobject* mur1 = nullptr;
        GLobject* mur2 = nullptr;

        Scene() = default;

        void import(const std::string&);
        void import_metro(const std::string& body, const std::string& left_doors, const std::string& right_doors);

        void init(Camera camera);
        void toggle_doors();
        bool update_doors();

        void draw();
    };
}
