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

        void update_camera();

        public:
        MetroGL() = default;

        void setup();

        void display();

        void handle_special_key(int key);

    };
}
