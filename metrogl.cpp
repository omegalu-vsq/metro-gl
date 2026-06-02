#include "metrogl.hh"

namespace gl {
    void MetroGL::setup() {
        scene_.import("blender/station_vide.glb");

        camera_.lookat(14.0f, 3.0f, 30.0f, 14.0f, 3.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        camera_.frustrum(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 500.0f);

        scene_.init_POV(camera_);
    }

    void MetroGL::display() {
        scene_.draw();
    }

}
