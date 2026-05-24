#pragma once

#include "vector.hh"

#include <vector>
#include <GL/glut.h>

namespace gl {
    class matrix4 {
        std::vector<GLfloat> mat;
        public:

        matrix4();

        GLfloat get(int im, int j);

        void insert_line(Vector3& vec, int j);

        void matrix4::insert_mat3(std::vector<Vector3>& mat);

        void matrix4::insert_mat4(std::vector<std::vector<GLfloat>>& mat);

        void operator*=(const matrix4& rhs);

        friend std::ostream& operator<<(std::ostream& out, const matrix4& op);
    };

}
