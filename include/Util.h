#pragma once

#include "Constants.h"

class Interval {
public:
    float m_Min, m_Max;

    Interval() : m_Min(+infinity_f32), m_Max(-infinity_f32) {}

    Interval(float min, float max) : m_Min(min), m_Max(max) {}

    float size() const {
        return m_Max - m_Min;
    }

    bool contains(float x) const {
        return m_Min <= x && x <= m_Max;
    }

    bool contains_open(float x) const {
        return m_Min < x && x < m_Max;
    }

    float clamp(float x) const {
        if (x < m_Min) return m_Min;
        if (x > m_Max) return m_Max;
        return x;
    }

    static const Interval empty, universe;
};

const Interval Interval::empty = Interval(+infinity_f32, -infinity_f32);
const Interval Interval::universe = Interval(-infinity_f32, +infinity_f32);

inline float linearToGamma(float linearComponent) {
    if (linearComponent > 0.0f) return std::sqrt(linearComponent);
    return 0.0f;
}

void writeColor(std::ostream &out, const color &pixel_color) {
    float r = pixel_color.x;
    float g = pixel_color.y;
    float b = pixel_color.z;

    // gamma transform with gamma = 2
    r = linearToGamma(r);
    g = linearToGamma(g);
    b = linearToGamma(b);

    static const Interval intensity(0.0f, 1 - 1e-3f);
    int rbyte = int(256.0f * intensity.clamp(r));
    int gbyte = int(256.0f * intensity.clamp(g));
    int bbyte = int(256.0f * intensity.clamp(b));

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

class Ray {
public:
    Ray() {}

    Ray(const point3 &origin, const vec3 &direction) : m_Origin(origin), m_Dir(direction) {}

    point3 at(float t) const {
        return m_Origin + t * m_Dir;
    }

    [[nodiscard]] inline DEF getDir() const -> vec3 {
        return m_Dir;
    }

    [[nodiscard]] inline DEF getOrigin() const -> vec3 {
        return m_Origin;
    }

private:
    point3 m_Origin;
    vec3 m_Dir;
};

class Material;
struct HitRecord {
    point3 p;
    vec3 n;
    shared_ptr<Material> material;
    float t;
    bool isFrontFace;

    void setFaceNormal(const Ray &r, const vec3 &outwardNormal) {
        isFrontFace = glm::dot(r.getDir(), outwardNormal) < 0.0f;
        n = isFrontFace ? outwardNormal : -outwardNormal;
    }
};