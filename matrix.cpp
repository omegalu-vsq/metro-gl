#include "matrix.hh"

#include <vector>

#include "vector.hh"

namespace gl {
    matrix4::matrix4() {
        mat.assign(4 * 4, 0.0f);
    }

    GLfloat* matrix4::data() {
        return mat.data();
    }

    GLfloat matrix4::get(int i, int j) const {
        return mat.at(j * 4 + i);
    }

    void matrix4::insert_line(Vector3& vec, int j) {
        for (int i = 0; i < 3; i++)
            mat[j * 4 + i] = vec[i];
    }

    void matrix4::insert_mat3(std::vector<Vector3>& mat) {
        for (int j = 0; j < 3; j++)
            for (int i = 0; i < 3; i++)
                this->mat[j * 4 + i] = mat[j][i];

        this->mat[3 * 4 + 3] = 1;
    }

    void matrix4::insert_mat4(std::vector<std::vector<GLfloat>>& mat) {
        for (int j = 0; j < 4; j++)
            for (int i = 0; i < 4; i++)
                this->mat[j * 4 + i] = mat[j][i];
    }

    void matrix4::operator*=(const matrix4& rhs) {
        std::vector<GLfloat> tmp(4 * 4, 0.0f);

        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                tmp[row * 4 + col] =
                    mat[row * 4 + 0] * rhs.mat[0 * 4 + col] +
                    mat[row * 4 + 1] * rhs.mat[1 * 4 + col] +
                    mat[row * 4 + 2] * rhs.mat[2 * 4 + col] +
                    mat[row * 4 + 3] * rhs.mat[3 * 4 + col];
            }
        }

        mat = tmp;
    }


    std::ostream& operator<<(std::ostream& out, const matrix4& op) {
        out << '[';

        for(int j = 0; j < 4; j++) {
            for (int i = 0; i < 4; i++) {
                out << op.get(i, j);

                if (i != 3)
                    out << ", ";
            }

            if (j != 4)
                out << ",\n";
        }

        out << ']' << std::endl;

        return out;
    }

    matrix4 matrix4::glmat() {
        matrix4 res;
        std::vector<std::vector<GLfloat>> resvec;

        for (int j = 0; j < 4; j++) {
            std::vector<GLfloat> line;

            for (int i = 0; i < 4; i++)
                line.push_back(get(j, i));

            resvec.push_back(line);
        }

        res.insert_mat4(resvec);

        return res;
    }



}
