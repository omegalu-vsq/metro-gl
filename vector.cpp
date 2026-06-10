#include "vector.hh"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace gl {

    // VECTOR 3
    Vector3::Vector3(GLfloat x, GLfloat y, GLfloat z) : x_(x), y_(y), z_(z)
    {}

    Vector3 operator+(const Vector3 &lhs, const Vector3 &rhs) {
        return Vector3(lhs.x_ + rhs.x_, lhs.y_ + rhs.y_, lhs.z_ + rhs.z_);
    }

    Vector3 operator-(const Vector3 &lhs, const Vector3 &rhs) {
        return Vector3(lhs.x_ - rhs.x_, lhs.y_ - rhs.y_, lhs.z_ - rhs.z_);
    }

    Vector3 Vector3::operator-() const {
        return Vector3(-x_, -y_, -z_);
    }

    GLfloat operator*(const Vector3 &lhs, const Vector3 &rhs) {
        return lhs.x_ * rhs.x_ + lhs.y_ * rhs.y_ + lhs.z_ * rhs.z_;
    }

    Vector3 operator*(const Vector3 &lhs, GLfloat scalar){
        return Vector3(lhs.x_ * scalar, lhs.y_ * scalar, lhs.z_ * scalar);
    }

    Vector3 operator*(GLfloat scalar, const Vector3 &rhs) {
        return Vector3(rhs.x_ * scalar, rhs.y_ * scalar, rhs.z_ * scalar);
    }

    Vector3 operator/(const Vector3 &lhs, GLfloat scalar) {
        return Vector3(lhs.x_ / scalar, lhs.y_ / scalar, lhs.z_ / scalar);
    }

    Vector3 operator^(const Vector3 &lhs, const Vector3 &rhs) {
        GLfloat nx = lhs.y_ * rhs.z_ - lhs.z_ * rhs.y_;
        GLfloat ny = lhs.z_ * rhs.x_ - lhs.x_ * rhs.z_;
        GLfloat nz = lhs.x_ * rhs.y_ - lhs.y_ * rhs.x_;

        return Vector3(nx, ny, nz);
    }

    bool operator==(const Vector3 &lhs, const Vector3 &rhs) {
        return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_ && lhs.z_ == rhs.z_;
    }

    bool operator!=(const Vector3 &lhs, const Vector3 &rhs){
        return !(lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_ && lhs.z_ == rhs.z_);
    }

    Vector3& Vector3::operator=(const Vector3 &other) {
        this->x_ = other.x_;
        this->y_ = other.y_;
        this->z_ = other.z_;

        return *this;
    }

    GLfloat& Vector3::operator[](int i) {
        i = i % 3;
        if (i == 0) return this->x_;
        else if (i == 1) return this->y_;
        else return this->z_;
    }

    std::ostream& operator<<(std::ostream &out,Vector3 &vect) {
        out << '(' << vect.x_ << " ," << vect.y_ << " ," << vect.z_ << std::endl;
        return out;
    }

    void Vector3::normalize() {
        GLfloat norm = std::sqrt(x_*x_ + y_*y_ + z_*z_);
        x_ /= norm;
        y_ /= norm;
        z_ /= norm;
    }

    GLfloat Vector3::norm() {
        return std::sqrt(x_*x_ + y_*y_ + z_*z_);
    }

    GLfloat* Vector3::data() {
        static thread_local GLfloat arr[3];
        arr[0] = x_;
        arr[1] = y_;
        arr[2] = z_;
        return arr;
    } 

}
