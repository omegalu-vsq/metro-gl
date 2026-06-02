#pragma once

#include "camera.hh"
#include "scene.hh"

namespace gl {
    class MetroGL {

        Camera camera_;
        Scene scene_;

        public:
        MetroGL() = default;

        void setup();

        void display();

    };
}