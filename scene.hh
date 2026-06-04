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
        Texture(const std::string& src);
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
        GLobject(std::shared_ptr<ShaderProgram> shaders) : shaders_(std::move(shaders)), transform_(), state_(false) {}
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

    class Scene {
        public:

        std::vector<std::unique_ptr<GLobject>> station_obj;
        std::vector<std::unique_ptr<GLobjectTex>> station_signals;
        
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

        void init(Camera camera);

        void draw();
    };
}
