#pragma once

#include "lve_model.hpp"
//libs
#include "glm/gtc/matrix_transform.hpp"
#include "Colision.hpp"

//Std
#include <memory>
#include <unordered_map>

namespace lve {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f,1.f,1.f };
        glm::vec3  rotation;
        glm::vec3 vitesse{ 0.0f,0.0f,0.0f };
        glm::vec3 acceleration{ 0.0f,0.0f,0.0f };
        float friction = 1.0f;

        AABB colisionBox = AABB();

        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
        void setTransform(glm::vec3 translation, glm::vec3 scale);
        void setTranslation(glm::vec3 translation);
        void update();
        void updateAcceleration();
        void bouncingAABB(AABB box);
    };

    struct PointLightComponent {
        float lightIntensity = 1.0f;
    };

    class LveGameObject {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, LveGameObject>;

        static LveGameObject createGameObject() { static id_t currentId = 0; return LveGameObject{ currentId++ }; }
        static LveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

        LveGameObject(const LveGameObject&) = delete;
        LveGameObject& operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject& operator=(LveGameObject&&) = default;

        id_t getId() { return id; }
        glm::vec3 get_point_box_min() { return { -transform.translation.x / 2, -transform.translation.y / 2, -transform.translation.z / 2 }; }
        glm::vec3 get_point_box_max() { return { transform.translation.x / 2, transform.translation.y / 2, transform.translation.z / 2 }; }

        std::shared_ptr<LveModel>model{};
        glm::vec3 color{};
        TransformComponent transform{};
        std::unique_ptr<PointLightComponent> pointLight = nullptr;


    private:
        LveGameObject(id_t objId) : id(objId) {}

        id_t id;
    };
}