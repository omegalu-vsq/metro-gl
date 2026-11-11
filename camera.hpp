#pragma once

#include "matrix.hpp"

#include <GL/glew.h>

namespace gl {

    class Camera {
        public:
        Vector3 pos;
        matrix4 view;
        matrix4 projection;

        Camera() = default;

        void setPos(GLfloat x, GLfloat y, GLfloat z) { pos.x_ = x; pos.y_ = y; pos.z_ = z; }

        void lookat(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);

        void frustrum(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);

    };

}
