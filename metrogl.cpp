#include "metrogl.hh"

#include <cmath>
#include <GL/freeglut.h>

namespace gl {
    void MetroGL::setup() {
        scene_.import("blender/station_vide.glb");

        update_camera();
    }

    void MetroGL::display() {
        scene_.draw();
    }

    void MetroGL::handle_special_key(int key) {
        const GLfloat move_step = 1.0f;
        const GLfloat turn_step = 0.08f;

        if (key == GLUT_KEY_LEFT) yaw_ += turn_step;
        if (key == GLUT_KEY_RIGHT) yaw_ -= turn_step;
        if (key == GLUT_KEY_UP) {
            camera_x_ += std::sin(yaw_) * move_step;
            camera_z_ -= std::cos(yaw_) * move_step;
        }
        if (key == GLUT_KEY_DOWN) {
            camera_x_ -= std::sin(yaw_) * move_step;
            camera_z_ += std::cos(yaw_) * move_step;
        }

        update_camera();
    }

    void MetroGL::update_camera() {
        camera_ = Camera{};

        camera_.lookat(camera_x_, camera_y_, camera_z_,
                       camera_x_ + std::sin(yaw_), camera_y_, camera_z_ - std::cos(yaw_),
                       0.0f, -1.0f, 0.0f);
        camera_.frustrum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 500.0f);

        scene_.init_POV(camera_);
    }

}
