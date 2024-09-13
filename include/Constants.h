#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#define DEF auto

using point3 = glm::vec3;
using color = glm::vec3;

using glm::vec3;
using std::cout;
using std::make_shared;
using std::optional;
using std::shared_ptr;

constexpr float infinity_f32 = std::numeric_limits<float>::infinity();

constexpr float pi = 3.1415926535897932385f;

constexpr size_t image_width = 256;
constexpr size_t image_height = 256;

constexpr vec3 CAMERA_FORWARD = vec3(0.0f, 0.0f, -1.0f);

inline float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0f;
}

#endif // CONSTANTS_H
