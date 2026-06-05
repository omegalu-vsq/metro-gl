#include "metrogl.hh"

#include <cmath>
#include <GL/freeglut.h>

namespace gl {
    void MetroGL::setup() {
        scene_.import("blender/station_vide.glb");
        scene_.import_metro("blender/mf77body.glb", "blender/mf77doorleft.glb", "blender/mf77doorright.glb");

        update_camera();
    }

    void MetroGL::display() {
        scene_.draw();
    }

    void MetroGL::handle_key(unsigned char key) {
        if (key == 'd') move_left_ = true;
        if (key == 'q') move_right_ = true;
        if (key == 'z') move_forward_ = true;
        if (key == 's') move_backward_ = true;
        if (key == 'l') toggle_mouse_lock();
        if (key == 'o') scene_.toggle_doors();
        if (key == 'r') scene_.run_metro();
    }

    void MetroGL::handle_key_up(unsigned char key) {
        if (key == 'd') move_left_ = false;
        if (key == 'q') move_right_ = false;
        if (key == 'z') move_forward_ = false;
        if (key == 's') move_backward_ = false;
    }

    void MetroGL::handle_mouse_motion(int x, int) {
        const GLfloat mouse_sensitivity = 0.003f;

        if (!mouse_locked_) return;

        int center_x = window_width_ / 2;
        int center_y = window_height_ / 2;

        if (x == center_x) return;

        yaw_ -= (x - center_x) * mouse_sensitivity;
        glutWarpPointer(center_x, center_y);
        update_camera();
    }

    void MetroGL::resize(int width, int height) {
        if (height == 0) return;

        window_width_ = width;
        window_height_ = height;
        aspect_ = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);
        update_camera();
    }

    void MetroGL::toggle_mouse_lock() {
        mouse_locked_ = !mouse_locked_;
        has_last_mouse_x_ = false;

        if (mouse_locked_) {
            glutSetCursor(GLUT_CURSOR_NONE);
            glutWarpPointer(window_width_ / 2, window_height_ / 2);
        }
        else {
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
    }

    bool MetroGL::update_controls() {
        const GLfloat move_step = 0.12f;
        bool moved = false;
        bool redraw = false;

        if (move_left_) {
            camera_x_ -= std::cos(yaw_) * move_step;
            camera_z_ -= std::sin(yaw_) * move_step;
            moved = true;
        }
        if (move_right_) {
            camera_x_ += std::cos(yaw_) * move_step;
            camera_z_ += std::sin(yaw_) * move_step;
            moved = true;
        }
        if (move_forward_) {
            camera_x_ += std::sin(yaw_) * move_step;
            camera_z_ -= std::cos(yaw_) * move_step;
            moved = true;
        }
        if (move_backward_) {
            camera_x_ -= std::sin(yaw_) * move_step;
            camera_z_ += std::cos(yaw_) * move_step;
            moved = true;
        }

        if (scene_.update_metro())
            redraw = true;

        if (moved)
            update_camera();

        return moved || redraw;
    }

    void MetroGL::update_camera() {
        camera_.lookat(camera_x_, camera_y_, camera_z_,
                       camera_x_ + std::sin(yaw_), camera_y_, camera_z_ - std::cos(yaw_),
                       0.0f, -1.0f, 0.0f);
        camera_.frustrum(-0.1f * aspect_, 0.1f * aspect_, -0.1f, 0.1f, 0.1f, 500.0f);

        scene_.init(camera_);
    }

}
