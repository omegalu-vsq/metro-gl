#pragma once

#include <GL/ut.h>

namespace gl {

    class Camera {
        public:
        matrix4 proj;

        Camera() = default;

        void lookat(GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&);

        void frustrum(GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&, GLfloat&);

    };

}
