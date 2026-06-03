#pragma once

#include "camera.hh"
#include "scene.hh"

namespace gl {
    class MetroGL {

        Camera camera_;
        Scene scene_;

        GLfloat camera_x_ = 14.0f;
        GLfloat camera_y_ = 3.0f;
        GLfloat camera_z_ = 30.0f;
        GLfloat yaw_ = 0.0f;
        GLfloat aspect_ = 16.0f / 9.0f;
        int last_mouse_x_ = 0;
        bool has_last_mouse_x_ = false;
        int window_width_ = 1920;
        int window_height_ = 1080;
        bool mouse_locked_ = false;
        bool move_forward_ = false;
        bool move_backward_ = false;
        bool move_left_ = false;
        bool move_right_ = false;

        void update_camera();
        void toggle_mouse_lock();

        public:
        MetroGL() = default;

        void setup();

        void display();

        void handle_special_key(int key);

        void handle_special_key_up(int key);

        void handle_key(unsigned char key);

        void handle_key_up(unsigned char key);

        void handle_mouse_motion(int x, int y);

        void resize(int width, int height);

        bool update_controls();

    };
}
