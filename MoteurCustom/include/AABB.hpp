#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include "Sphere.hpp"

namespace lve {
    //AABB = axis-aligned bounding box
    class AABB {
    public:
        float minX;
        float maxX;
        float minY;
        float maxY;
        float minZ;
        float maxZ;

        //Constructeur par défaut
        AABB() {
            this->minX = 0.0;
            this->maxX = 0.0;

            this->minY = 0.0;
            this->maxY = 0.0;

            this->minZ = 0.0;
            this->maxZ = 0.0;
        }

        //Constructeur avec points
        AABB(glm::vec3 pointA, glm::vec3 pointB) {
            this->minX = (pointA.x < pointB.x) ? pointA.x : pointB.x;
            this->maxX = (pointA.x > pointB.x) ? pointA.x : pointB.x;

            this->minY = (pointA.y < pointB.y) ? pointA.y : pointB.y;
            this->maxY = (pointA.y > pointB.y) ? pointA.y : pointB.y;

            this->minZ = (pointA.z < pointB.z) ? pointA.z : pointB.z;
            this->maxZ = (pointA.z > pointB.z) ? pointA.z : pointB.z;
        }

        //Constructeur avec valeurs direct
        AABB(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
            this->minX = minX;
            this->maxX = maxX;

            this->minY = minY;
            this->maxY = maxY;

            this->minZ = minZ;
            this->maxZ = maxZ;
        }

        //Constructeur par copie
        AABB(const AABB& box) {
            this->minX = box.minX;
            this->maxX = box.maxX;

            this->minY = box.minY;
            this->maxY = box.maxY;

            this->minZ = box.minZ;
            this->maxZ = box.maxZ;
        }

        void setBoxPoint(glm::vec3 pointA, glm::vec3 pointB) {
            this->minX = (pointA.x < pointB.x) ? pointA.x : pointB.x;
            this->maxX = (pointA.x > pointB.x) ? pointA.x : pointB.x;

            this->minY = (pointA.y < pointB.y) ? pointA.y : pointB.y;
            this->maxY = (pointA.y > pointB.y) ? pointA.y : pointB.y;

            this->minZ = (pointA.z < pointB.z) ? pointA.z : pointB.z;
            this->maxZ = (pointA.z > pointB.z) ? pointA.z : pointB.z;
        }

        //Sphère contre AABB
        bool isIntersectSphere(Sphere sphere) {
            float x = this->max(this->minX, this->min(sphere.x, this->maxX));
            float y = this->max(this->minY, this->min(sphere.y, this->maxY));
            float z = this->max(this->minZ, this->min(sphere.z, this->maxZ));

            float distance = sqrt(
                (x - sphere.x) * (x - sphere.x) +
                (y - sphere.y) * (y - sphere.y) +
                (z - sphere.z) * (z - sphere.z)
            );

            return distance < sphere.radius;
        }

        //calcule le point le plus proche de la sphère sur le cube AABB et utilise cette information pour déterminer la normale de collision. Elle renvoie la direction dans laquelle la sphère devrait se déplacer après la collision avec le cube.
        glm::vec3 normIntersectSphere(Sphere sphere) {
            glm::vec3 norm = { 0.f, 0.f, 0.f };

            // Calcul du point le plus proche de la sphère sur le cube
            float x = glm::clamp(sphere.x, this->minX, this->maxX);
            float y = glm::clamp(sphere.y, this->minY, this->maxY);
            float z = glm::clamp(sphere.z, this->minZ, this->maxZ);

            // Calcul de la normale basée sur le point le plus proche
            norm.x = sphere.x - x;
            norm.y = sphere.y - y;
            norm.z = sphere.z - z;

            // Normalisation de la normale
            norm = glm::normalize(norm);

            return norm;
        }

        //AABB contre AABB
        bool isIntersectAABB(AABB box) {
            return (
                box.minX <= this->maxX &&
                box.maxX >= this->minX &&
                box.minY <= this->maxY &&
                box.maxY >= this->minY &&
                box.minZ <= this->maxZ &&
                box.maxZ >= this->minZ
                );
        }

        //détermine la quelle valeur inverser lors d'une colision entre deux cubes
        glm::vec3 normIntersectAABB(AABB box) {
            glm::vec3 norm = { 1.f, 1.f, 1.f };

            if (isIntersectAABB(box)) {
                float xOverlap = std::min(this->maxX, box.maxX) - std::max(this->minX, box.minX);
                float yOverlap = std::min(this->maxY, box.maxY) - std::max(this->minY, box.minY);
                float zOverlap = std::min(this->maxZ, box.maxZ) - std::max(this->minZ, box.minZ);

                if (xOverlap < yOverlap && xOverlap < zOverlap) {
                    //Face touchée : face X
                    norm = { -1.f, 1.f, 1.f };
                } else if (yOverlap < xOverlap && yOverlap < zOverlap) {
                    //Face touchée : face Y
                    norm = { 1.f, -1.f, 1.f };
                } else {
                    //Face touchée : face Z
                    norm = { 1.f, 1.f, -1.f };
                }
            }

            return norm;
        }

        //Point contre AABB
        bool isPointInside(glm::vec3 point) { //AABB = axis-aligned bounding box
            return (
                point.x >= this->minX &&
                point.x <= this->maxX &&
                point.y >= this->minY &&
                point.y <= this->maxY &&
                point.z >= this->minZ &&
                point.z <= this->maxZ
                );
        }

    private:
        //retourne la plus petite valeur entre a et b
        float min(float a, float b) {
            float res = a;

            if (a > b) {
                res = b;
            }

            return res;
        }
        //retourne la plus grande valeur entre a et b
        float max(float a, float b) {
            float res = b;

            if (a >= b) {
                res = a;
            }

            return res;
        }

        //renvois les coordonées du point au centre de la boite de colision
        glm::vec3 center() {
            return { minX + ((minX - maxX) / 2), minY + ((minY - maxY) / 2), minZ + ((minZ - maxZ) / 2) };
        }
    };
}