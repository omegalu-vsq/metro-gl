#pragma once

#include "matrix.hh"

#include <GL/glew.h>

namespace gl {

    class Camera {
        public:
        matrix4 model_view;
        matrix4 projection;

        Camera() = default;

        void lookat(GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&);

        void frustrum(GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&);

    };

}
