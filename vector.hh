#pragma once
#include <cstdlib>
#include <iostream>

#include <GL/ut.h>

namespace ray {
    class Vector3 {
        public:
            GLfloat x_;
            GLfloat y_;
            GLfloat z_;

            Vector3() = default;
            Vector3(GLfloat x, GLfloat y, GLfloat z);
            ~Vector3() = default;

            friend Vector3 operator+(const Vector3 &lhs, const Vector3 &rhs);
            friend Vector3 operator-(const Vector3 &lhs, const Vector3 &rhs);
            friend GLfloat operator*(const Vector3 &lhs, const Vector3 &rhs);
            friend Vector3 operator*(const Vector3 &lhs, GLfloat scalar);
            friend Vector3 operator*(GLfloat scalar, const Vector3 &rhs);
            friend Vector3 operator/(const Vector3 &lhs, GLfloat scalar);
            friend Vector3 operator^(const Vector3 &lhs, const Vector3 &rhs);
            friend bool operator==(const Vector3 &lhs, const Vector3 &rhs);
            friend bool operator!=(const Vector3 &lhs, const Vector3 &rhs);
            Vector3 operator-() const;
            Vector3& operator=(const Vector3 &other);
            GLfloat& operator[](int i);

            void normalize();
            GLfloat norm();

            friend std::ostream& operator<<(std::ostream &out,Vector3 &vect);
    };

    /*
    class Vector4 {
        public:
            GLfloat x_;
            GLfloat y_;
            GLfloat z_;
            GLfloat w_;

            Vector4(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
            ~Vector4();

            friend Vector4 operator+(const Vector4 &lhs, const Vector4 &rhs);
            friend Vector4 operator-(const Vector4 &lhs, const Vector4 &rhs);
            friend Vector4 operator*(const Vector4 &lhs, const Vector4 &rhs);
            friend Vector4 operator*(const Vector4 &lhs, GLfloat scalar);
            friend Vector4 operator*(GLfloat scalar, const Vector4 &rhs);
            friend Vector4 operator^(const Vector4 &lhs, const Vector4 &rhs);

            friend std::ostream& operator<<(std::ostream &out,Vector4 &vect);
    };
    */
}
