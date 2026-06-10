#pragma once

#include "vector.hh"

#include <vector>
#include <GL/glew.h>

namespace gl {
    class matrix4 {
        std::vector<GLfloat> mat;
        public:

        matrix4();

        GLfloat* data();

        GLfloat get(int im, int j) const;

        void insert_line(Vector3& vec, int j);
        void insert_mat3(std::vector<Vector3>& mat);
        void insert_mat4(std::vector<std::vector<GLfloat>>& mat);

        void translation(GLfloat x, GLfloat y, GLfloat z);
        
        matrix4 glmat();

        void operator*=(const matrix4& rhs);

        friend std::ostream& operator<<(std::ostream& out, const matrix4& op);
    };

}
