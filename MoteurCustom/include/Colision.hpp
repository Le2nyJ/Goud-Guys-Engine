#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>

//#include "lve_model.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "AABB.hpp"
#include "Sphere.hpp"

namespace lve {
    class Colision {
    public:
        //Point contre AABB
        bool isPointInsideAABB(glm::vec3 point, AABB box) { //AABB = axis-aligned bounding box
            return (
                point.x >= box.minX &&
                point.x <= box.maxX &&
                point.y >= box.minY &&
                point.y <= box.maxY &&
                point.z >= box.minZ &&
                point.z <= box.maxZ
                );
        }

        //Point contre sphère
        bool isPointInsideSphere(glm::vec3 point, Sphere sphere) {
            float distance = sqrt(
                (point.x - sphere.x) * (point.x - sphere.x) +
                (point.y - sphere.y) * (point.y - sphere.y) +
                (point.z - sphere.z) * (point.z - sphere.z)
            );
            return distance < sphere.radius;
        }

        //AABB contre AABB
        bool isIntersectAABB2(AABB boxA, AABB boxB) {
            return (
                boxA.minX <= boxB.maxX &&
                boxA.maxX >= boxB.minX &&
                boxA.minY <= boxB.maxY &&
                boxA.maxY >= boxB.minY &&
                boxA.minZ <= boxB.maxZ &&
                boxA.maxZ >= boxB.minZ
                );
        }

        //Sphère contre sphère
        bool isIntersectSphere2(Sphere sphereA, Sphere sphereB) {
            float distance = sqrt(
                (sphereA.x - sphereB.x) * (sphereA.x - sphereB.x) +
                (sphereA.y - sphereB.y) * (sphereA.y - sphereB.y) +
                (sphereA.z - sphereB.z) * (sphereA.z - sphereB.z)
            );
            return distance < sphereA.radius + sphereB.radius;
        }

        //Sphère contre AABB
        bool isIntersectSphereAABB(Sphere sphere, AABB box) {
            float x = this->max(box.minX, this->min(sphere.x, box.maxX));
            float y = this->max(box.minY, this->min(sphere.y, box.maxY));
            float z = this->max(box.minZ, this->min(sphere.z, box.maxZ));

            float distance = sqrt(
                (x - sphere.x) * (x - sphere.x) +
                (y - sphere.y) * (y - sphere.y) +
                (z - sphere.z) * (z - sphere.z)
            );

            return distance < sphere.radius;
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
    };
}