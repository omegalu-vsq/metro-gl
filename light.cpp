#include "light.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <utility>

namespace gl {

    std::pair<GLlightTube, std::unique_ptr<GLobject>> GLlightTube::make_light_tube(Vector3 color, Vector3 pos, float intensity, float radius, float length, std::shared_ptr<ShaderProgram> shaders, Vector3 tangent) {
        // geometry construction
        const unsigned int segments = 24;
        const float pi = 3.14159265f;
        const unsigned int bottom = 4 * segments;
        const unsigned int top = bottom + 1;

        aiMesh mesh;
        mesh.mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
        mesh.mNumVertices = 4 * segments + 2;
        mesh.mNumFaces = 4 * segments;
        mesh.mVertices = new aiVector3D[mesh.mNumVertices];
        mesh.mNormals = new aiVector3D[mesh.mNumVertices];
        mesh.mFaces = new aiFace[mesh.mNumFaces];

        mesh.mVertices[bottom] = aiVector3D(0.0f, 0.0f, -length / 2.0f);
        mesh.mVertices[top] = aiVector3D(0.0f, 0.0f, length / 2.0f);
        mesh.mNormals[bottom] = aiVector3D(0.0f, 0.0f, -1.0f);
        mesh.mNormals[top] = aiVector3D(0.0f, 0.0f, 1.0f);

        for (unsigned int i = 0; i < segments; i++) {
            float angle = 2.0f * pi * i / segments;
            aiVector3D normal(std::cos(angle), std::sin(angle), 0.0f);
            mesh.mVertices[i] = aiVector3D(radius * normal.x, radius * normal.y, -length / 2.0f);
            mesh.mVertices[i + segments] = aiVector3D(radius * normal.x, radius * normal.y, length / 2.0f);
            mesh.mNormals[i] = mesh.mNormals[i + segments] = normal;

            mesh.mVertices[i + 2 * segments] = mesh.mVertices[i];
            mesh.mVertices[i + 3 * segments] = mesh.mVertices[i + segments];
            mesh.mNormals[i + 2 * segments] = mesh.mNormals[bottom];
            mesh.mNormals[i + 3 * segments] = mesh.mNormals[top];

            unsigned int next = (i + 1) % segments;
            unsigned int triangles[4][3] = {
                {i, next, i + segments},
                {next, next + segments, i + segments},
                {bottom, next + 2 * segments, i + 2 * segments},
                {top, i + 3 * segments, next + 3 * segments}
            };

            for (unsigned int j = 0; j < 4; j++) {
                aiFace& face = mesh.mFaces[4 * i + j];
                face.mNumIndices = 3;
                face.mIndices = new unsigned int[3]{triangles[j][0], triangles[j][1], triangles[j][2]};
            }
        }
       
        tangent = tangent.norm() > 0.0f ? tangent : Vector3{0.0f, 0.0f, 1.0f};
        tangent.normalize();
        Vector3 up = std::abs(tangent.y_) < 0.99f ? Vector3{0.0f, 1.0f, 0.0f} : Vector3{1.0f, 0.0f, 0.0f};
        Vector3 right = up ^ tangent;
        right.normalize();
        up = tangent ^ right;

        for (unsigned int i = 0; i < mesh.mNumVertices; i++) {
            const aiVector3D vertex = mesh.mVertices[i];
            const aiVector3D normal = mesh.mNormals[i];
            Vector3 p = pos + right * vertex.x + up * vertex.y + tangent * vertex.z;
            Vector3 n = right * normal.x + up * normal.y + tangent * normal.z;
            mesh.mVertices[i] = aiVector3D(p.x_, p.y_, p.z_);
            mesh.mNormals[i] = aiVector3D(n.x_, n.y_, n.z_);
        }

        std::unique_ptr<GLobject> obj = std::make_unique<GLobject>(GLobject::make_object(&mesh, std::move(shaders)));
        
        GLlightTube tube{color, pos, intensity, obj.get(), radius, length, tangent};
        return {tube, std::move(obj)};
    }

    void GLlightTube::init_uniform(GLuint shader_id, int idx) {
        GLint light_color_id, light_intensity_id, light_radius_id;
        GLint light_points_id;

        auto tstr = std::string("tubelights[") + std::to_string(idx) + std::string("]");

        light_color_id = glGetUniformLocation(shader_id, (tstr + ".color").c_str());
        light_intensity_id = glGetUniformLocation(shader_id, (tstr + ".intensity").c_str());
        light_radius_id = glGetUniformLocation(shader_id, (tstr + ".radius").c_str());
        light_points_id = glGetUniformLocation(shader_id, (tstr + ".points").c_str());

        Vector3 linear_color = color_;
        for (int i = 0; i < 3; i++) {
            float c = color_[i];
            linear_color[i] = c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
        }
        glUniform3fv(light_color_id, 1, linear_color.data());
        glUniform1f(light_intensity_id, this->intensity_);
        glUniform1f(light_radius_id, this->radius_);

        std::vector<Vector3> points;
        points.push_back(position_ - 0.5f * length_ * tangent_);
		points.push_back(position_ + 0.5f * length_ * tangent_);

        std::vector<GLfloat> GLpts;
        for (int i = 0; i < 2; i++) {
            GLpts.push_back(points[i].x_);
            GLpts.push_back(points[i].y_);
            GLpts.push_back(points[i].z_);
        }

        glUniform3fv(light_points_id, 2, GLpts.data());
    }

    const int shadow_size = 512;
    const float shadow_far_plane = 80.0f;

    ShadowController::ShadowController(const std::vector<Vector3>& positions) : positions_(positions) {
        if (positions_.empty()) return;

        GLint max_layers;
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);

        if (positions_.size() * 6 > static_cast<size_t>(max_layers)) {
            std::cerr << "/!\\ ShadowController creation failure /!\\ (too many cubemap layers)\n";
            return;
        }

        shader_ = std::make_unique<ShaderProgram>(ShaderProgram::make_program("shaders/shadow.vert", "shaders/shadow.frag", "shaders/shadow.geo"));
        if (!shader_->is_ready()) return;

        GLint active_texture;
        glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture);

        glActiveTexture(GL_TEXTURE10);
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture_);

        glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT24, shadow_size, shadow_size, positions_.size() * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glActiveTexture(active_texture);

        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &saved_draw_framebuffer_);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &saved_read_framebuffer_);

        glGenFramebuffers(1, &framebuffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        ready_ = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, saved_draw_framebuffer_);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, saved_read_framebuffer_);

        if (!ready_) {
            std::cerr << "/!\\ ShadowController creation failure /!\\ (incomplete framebuffer)\n";
            return;
        }

        Vector3 directions[] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
        Vector3 up[] = {{0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}};

        for (size_t light = 0; light < positions_.size(); light++) {
            Vector3 position = positions_[light];
            center_x_ += position.x_;

            for (int face = 0; face < 6; face++) {
                Camera camera;

                Vector3 right = directions[face] ^ up[face];
                Vector3 vertical = right ^ directions[face];
                std::vector<Vector3> rotation{right, vertical, -directions[face]};

                camera.view.insert_mat3(rotation);
                camera.view.translation(-position.x_, -position.y_, -position.z_);
                camera.frustrum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, shadow_far_plane);

                matrix4 transform = camera.projection;
                transform *= camera.view;

                matrices_.insert(matrices_.end(), transform.data(), transform.data() + 16);
            }
        }

        center_x_ /= positions_.size();

        shader_->use();
        glUniform1f(glGetUniformLocation(shader_->getId(), "shadow_far_plane"), shadow_far_plane);
    }

    ShadowController::~ShadowController() {
        if (framebuffer_) glDeleteFramebuffers(1, &framebuffer_);
        if (texture_) glDeleteTextures(1, &texture_);
    }

    void ShadowController::begin() {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &saved_draw_framebuffer_);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &saved_read_framebuffer_);
        glGetIntegerv(GL_VIEWPORT, saved_viewport_);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &saved_depth_mask_);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
        glViewport(0, 0, shadow_size, shadow_size);
        glDepthMask(GL_TRUE);

        glClear(GL_DEPTH_BUFFER_BIT);

        shader_->use();
    }

    void ShadowController::select_light(size_t index) {
        Vector3& position = positions_.at(index);

        shader_->use();

        glUniformMatrix4fv(glGetUniformLocation(shader_->getId(), "shadow_matrices"), 6, GL_TRUE, matrices_.data() + index * 96);
        glUniform3fv(glGetUniformLocation(shader_->getId(), "shadow_light_position"), 1, position.data());
        glUniform1i(glGetUniformLocation(shader_->getId(), "shadow_base_layer"), index * 6);
        glUniform1i(glGetUniformLocation(shader_->getId(), "shadow_wall_face"), position.x_ > center_x_ ? 0 : 1);
    }

    void ShadowController::end() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, saved_draw_framebuffer_);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, saved_read_framebuffer_);

        glViewport(saved_viewport_[0], saved_viewport_[1], saved_viewport_[2], saved_viewport_[3]);
        glDepthMask(saved_depth_mask_);
    }

    void ShadowController::apply(ShaderProgram& receiver, bool enabled) {
        receiver.use();

        glUniform1i(glGetUniformLocation(receiver.getId(), "shadow_map"), 10);
        glUniform1i(glGetUniformLocation(receiver.getId(), "shadow_count"), enabled ? positions_.size() : 0);
        glUniform1f(glGetUniformLocation(receiver.getId(), "shadow_far_plane"), shadow_far_plane);
    }
}
