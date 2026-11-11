#include "light.hpp"

#include <cmath>
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
}
