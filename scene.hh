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
    class GLobject {
        ShaderProgram shaders_;
        GLuint vao_id_;
        matrix4 transform_;
        GLuint nb_vertices_;
        GLuint nb_indices_;

        bool state_;

        public:
        GLobject(ShaderProgram&& shaders) : shaders_(std::move(shaders)), transform_(), state_(false) {}

        void init(Camera camera);

        void draw(Vector3 color);

        static GLobject make_object(const aiMesh* src, ShaderProgram&& shaders);
    };

    class Scene {
        public:

        std::vector<std::unique_ptr<GLobject>> station_obj;
        
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

        void init_POV(Camera camera);

        void draw();
    };
}
