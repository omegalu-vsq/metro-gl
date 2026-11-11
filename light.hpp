#pragma once

#include "object.hpp"
#include "program.hpp"
#include "vector.hpp"

#include <vector>
#include <optional>

namespace gl {
    class GLlight {
        protected:
        Vector3 color_;
        Vector3 position_;
        float intensity_;

        GLobject* object_ = nullptr;

        public:
        GLlight(Vector3 color, Vector3 pos, float intensity, GLobject* object = nullptr) : color_(color), position_(pos), intensity_(intensity), object_(object) {}
        virtual ~GLlight() = default; 

        GLobject* getObj() { return object_; }

        void draw() {
            if (object_) object_->draw(color_);
        }

        virtual void init_uniform(GLuint shader_id, int idx) = 0;

    };

    class GLlightTube : public GLlight {
        float radius_;
        float length_;
        Vector3 tangent_;

        public:
        GLlightTube(Vector3 color, Vector3 pos, float intensity, GLobject* object, float radius, float length, Vector3 tangent)
            : GLlight(color, pos, intensity, object), radius_(radius), length_(length), tangent_(tangent) {}

        static std::pair<GLlightTube, std::unique_ptr<GLobject>> make_light_tube(Vector3 color, Vector3 pos, float intensity, float radius, float length, std::shared_ptr<ShaderProgram> shaders, Vector3 tangent = Vector3{0.0f, 0.0f, 1.0f});

        void init_uniform(GLuint shader_id, int idx) override;
    };
}
