#include "camera.hh"

#include <vector>

#include <GL/glew.h>

namespace gl {

    void Camera::lookat(GLfloat ex, GLfloat ey, GLfloat ez,
                GLfloat cx, GLfloat cy, GLfloat cz,
                GLfloat ux, GLfloat uy, GLfloat uz) {

        Vector3 front = {cx - ex, cy - ey, cz - ez};
        front.normalize();

        Vector3 up = {ux, uy, uz};
        up.normalize();

        Vector3 side = front ^ up;
        side.normalize();

        front = front * -1;

        Vector3 u = side ^ front;
        u.normalize();

        std::vector<Vector3> view_rotation;
        view_rotation.push_back(side);
        view_rotation.push_back(u);
        view_rotation.push_back(front);

        model_view.insert_mat3(view_rotation);

        model_view.translation(-ex, -ey, -ez);
    }

    void Camera::frustrum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
                  GLfloat near, GLfloat far) {

        GLfloat A = (right + left) / (right - left);
        GLfloat B = (top + bottom) / (top - bottom);
        GLfloat C = -((far + near) / (far - near));
        GLfloat D = -((2 * far * near) / (far - near));
        GLfloat E = 2 * near / (right - left);
        GLfloat F = 2 * near / (top - bottom);

        std::vector<std::vector<GLfloat>> matmat;

        std::vector<GLfloat> l1 = { E, 0, A, 0 };
        std::vector<GLfloat> l2 = { 0, F, B, 0 };
        std::vector<GLfloat> l3 = { 0, 0, C, D };
        std::vector<GLfloat> l4 = { 0, 0, -1, 0 };

        matmat.push_back(l1);
        matmat.push_back(l2);
        matmat.push_back(l3);
        matmat.push_back(l4);

        projection.insert_mat4(matmat);
    }
}
