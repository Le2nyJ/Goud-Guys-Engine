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

namespace lve {
    class Sphere {
    public:
        float x;
        float y;
        float z;
        float radius;

        Sphere() {
            this->x = 0.0;
            this->y = 0.0;
            this->z = 0.0;
            this->radius = 1.0;
        }

        Sphere(glm::vec3 point, float radius) {
            this->x = point.x;
            this->y = point.y;
            this->z = point.z;
            this->radius = radius;
        }

        Sphere(float x, float y, float z, float radius) {
            this->x = x;
            this->y = y;
            this->z = z;
            this->radius = radius;
        }

        Sphere(const Sphere& s) {
            this->x = s.x;
            this->y = s.y;
            this->z = s.z;
            this->radius = s.radius;
        }
    };
}