#pragma once

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

constexpr float SHADOW_ACNE_FIX_THRESHOLD = 1e-3f;

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

inline vec3 random_vec3() { return vec3(random_float(), random_float(), random_float()); }
inline vec3 random_vec3(float min, float max) { return vec3(random_float(min, max), random_float(min, max), random_float(min, max)); }

inline vec3 random_vec3_n() {
    size_t iter = 0;
    while (iter++ < 1000) {
        vec3 sample = random_vec3(-1.0f, 1.0f);
        float lengthSquared = glm::dot(sample, sample);
        if (1e-8 < lengthSquared && lengthSquared <= 1.0f) {
            return glm::normalize(sample);
        }
    }
    throw std::runtime_error("Ran through too many iterations in random_vec3_n!");
}

inline vec3 randomOnHemisphere(const vec3 &n) {
    vec3 sample = random_vec3_n();
    return glm::dot(sample, n) > 0.0f ? sample : -sample;
}

inline vec3 reflect(const vec3 &v, const vec3 &n) {
    return v - vec3(2.0f * glm::dot(v, n)) * n;
}

inline vec3 refract(const vec3 &uv, const vec3 &n, float etai_over_etat) {
    auto cos_theta = std::fmin(dot(-uv, n), 1.0f);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -vec3(std::sqrt(std::fabs(1.0 - glm::dot(r_out_perp, r_out_perp)))) * n;
    return r_out_perp + r_out_parallel;
}