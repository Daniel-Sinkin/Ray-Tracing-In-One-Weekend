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

constexpr vec3 CAMERA_FORWARD = vec3(0.0f, 0.0f, -1.0f);

constexpr vec3 ORIGIN = point3(0.0f);

constexpr color WHITE = color(1.0f);
constexpr color RED = color(1.0f, 0.0f, 0.0f);
constexpr color GREEN = color(0.0f, 1.0f, 0.0f);
constexpr color BLUE = color(0.0f, 0.0f, 1.0f);
constexpr color BLACK = color(0.0f);

inline float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0f;
}

inline float random_float() { return std::rand() / (static_cast<float>(RAND_MAX) + 1.0f); }
inline float random_float(float min, float max) { return min + (max - min) * random_float(); }

#endif // CONSTANTS_H
