#include "lve_game_object.hpp"

namespace lve {
    /// <summary>
    /// Retourne la matrice de transformation 4x4 basée sur la translation, l'échelle et la rotation de l'obje
    /// </summary>
    /// <returns></returns>
    glm::mat4 TransformComponent::mat4() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        return glm::mat4{
            {
                scale.x * (c1 * c3 + s1 * s2 * s3),
                scale.x * (c2 * s3),
                scale.x * (c1 * s2 * s3 - c3 * s1),
                0.0f,
            },
            {
                scale.y * (c3 * s1 * s2 - c1 * s3),
                scale.y * (c2 * c3),
                scale.y * (c1 * c3 * s2 + s1 * s3),
                0.0f,
            },
            {
                scale.z * (c2 * s1),
                scale.z * (-s2),
                scale.z * (c1 * c2),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f} };
    }
    /// <summary>
    ///  Retourne la matrice normale 3x3 basée sur l'inverse de l'échelle et la rotation de l'objet
    /// </summary>
    /// <returns></returns>
    glm::mat3 TransformComponent::normalMatrix() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
            {
                invScale.x * (c1 * c3 + s1 * s2 * s3),
                invScale.x * (c2 * s3),
                invScale.x * (c1 * s2 * s3 - c3 * s1),
            },
            {
                invScale.y * (c3 * s1 * s2 - c1 * s3),
                invScale.y * (c2 * c3),
                invScale.y * (c1 * c3 * s2 + s1 * s3),
            },
            {
                invScale.z * (c2 * s1),
                invScale.z * (-s2),
                invScale.z * (c1 * c2),
            }
        };
    }
    /// <summary>
    /// Modifie le scale et la position de l'obet ainsi que pour so hit box
    /// </summary>
    /// <param name="translation"></param>
    /// <param name="scale"></param>
    void TransformComponent::setTransform(glm::vec3 translation, glm::vec3 scale) {
        this->translation = translation;
        this->scale = scale;
        //Modification de la boite de colision en consequence
        colisionBox.setBoxPoint({ this->translation.x - this->scale.x / 2,
                                 this->translation.y - this->scale.y / 2,
                                 this->translation.z - this->scale.z / 2 },
                                { this->translation.x + this->scale.x / 2,
                                 this->translation.y + this->scale.y / 2,
                                 this->translation.z + this->scale.z / 2 });
    }
    /// <summary>
    /// Modifie la translation de l'objet ainsi que sa hitbox
    /// </summary>
    /// <param name="translation"></param>
    void TransformComponent::setTranslation(glm::vec3 translation) {
        this->translation = translation;
        //Modification de la boite de colision en consequence
        colisionBox.setBoxPoint({ this->translation.x - this->scale.x / 2,
                                 this->translation.y - this->scale.y / 2,
                                 this->translation.z - this->scale.z / 2 },
                                { this->translation.x + this->scale.x / 2,
                                 this->translation.y + this->scale.y / 2,
                                 this->translation.z + this->scale.z / 2 });
    }
    /// <summary>
    /// Applique l'acceleration a la vitesse et la vitesse a la position
    /// </summary>
    void TransformComponent::update() {
        this->vitesse += this->acceleration;
        this->translation += this->vitesse;
        setTranslation(this->translation);
    }
    
    //Permet de ralentir l'obj a chaque appelle de la fonction en fonction de la valeur de friction

    /// <summary>
    /// Applique l'acceleration a la vitesse et la vitesse a la position en prenant en compte les frictions
    /// </summary>
    void TransformComponent::updateAcceleration() {
        this->vitesse.x *= this->friction;
        this->vitesse.y *= this->friction;
        this->vitesse.z *= this->friction;
        //Bloque le déplacement de l'obj si sa vitesse est trop lente sur un axe
        this->vitesse.x = (this->vitesse.x < 0.001 && this->vitesse.x > -0.001) ? 0.0f : this->vitesse.x;
        this->vitesse.y = (this->vitesse.y < 0.001 && this->vitesse.y > -0.001) ? 0.0f : this->vitesse.y;
        this->vitesse.z = (this->vitesse.z < 0.001 && this->vitesse.z > -0.001) ? 0.0f : this->vitesse.z;
    }
    /// <summary>
    /// Applique un rebond a l'objet en fonction de la face touchée
    /// </summary>
    /// <param name="box"></param>
    void TransformComponent::bouncingAABB(AABB box) {
        this->vitesse *= this->colisionBox.normIntersectAABB(box);
    }
    /// <summary>
    /// Retourne la matrice de transformation 4x4 basée sur la translation, l'échelle et la rotation de l'obje
    /// </summary>
    /// <param name="intensity"></param>
    /// <param name="radius"></param>
    /// <param name="color"></param>
    /// <returns></returns>
    LveGameObject LveGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
        LveGameObject gameObj = LveGameObject::createGameObject();
        gameObj.color = color;
        gameObj.transform.scale.x = radius;
        gameObj.pointLight = std::make_unique<PointLightComponent>();
        gameObj.pointLight->lightIntensity = intensity;
        return gameObj;
    }
}