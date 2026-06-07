#pragma once

#include "object.hh"

#include <GL/glew.h>

#include <memory>
#include <string>
#include <vector>

namespace gl {
    class Metro {
        std::vector<std::unique_ptr<GLobjectMetro>> body_;
        std::vector<std::unique_ptr<GLobjectTexLight>> textures_;
        std::vector<std::unique_ptr<GLobjectMetro>> left_doors_;
        std::vector<std::unique_ptr<GLobjectMetro>> right_doors_;

        GLfloat door_opening_ = 0.0f;
        GLfloat target_door_opening_ = 0.0f;

        GLfloat z_position_ = 0.0f;
        GLfloat z_position_start_ = 0.0f;
        GLfloat z_position_target_ = 0.0f;

        void import_body(const std::string& src);
        void import_doors(const std::string& src, bool left);

        public:
        Metro() = default;

        void import(const std::string& body, const std::string& left_doors, const std::string& right_doors);
        void init(Camera camera);
        void init_metro_lights();
        void draw();

        void toggle_doors();
        void run();
        bool update_metro();
        GLfloat z_position() const { return z_position_; }
    };

    class Scene {
        public:

        std::vector<std::unique_ptr<GLobject>> station_obj;
        std::vector<std::unique_ptr<GLobjectTexLight>> station_signals;
        std::vector<std::unique_ptr<GLobjectTexLight>> station_plans;

        std::unique_ptr<Metro> metro;
        
        GLobject* quai1 = nullptr;
        GLobject* quai2 = nullptr;
        GLobject* sous_quai1 = nullptr;
        GLobject* sous_quai2 = nullptr;

        GLobject* voies = nullptr;
        std::vector<GLobject*> rails;
        std::vector<GLobject*> traverses;
        std::vector<GLobject*> tunnels;
        std::vector<GLobject*> bouchons;

        GLobject* voute = nullptr;
        GLobject* mur1 = nullptr;
        GLobject* mur2 = nullptr;

        std::vector<std::unique_ptr<GLobjectPodo>> podotactile_strips;

        Scene() = default;

        void import(const std::string&);
        void import_metro(const std::string& body, const std::string& left_doors, const std::string& right_doors);

        void init(Camera camera);
        void init_metro_lights();

        void toggle_doors();
        void run_metro();
        bool update_metro();

        void draw();
    };
}
