#ifndef UTIL_H
#define UTIL_H

#include "Constants.h"

class interval {
public:
    float min, max;

    interval() : min(+infinity_f32), max(-infinity_f32) {}

    interval(float min, float max) : min(min), max(max) {}

    float size() const {
        return max - min;
    }

    bool contains(float x) const {
        return min <= x && x <= max;
    }

    bool contains_open(float x) const {
        return min < x && x < max;
    }

    static const interval empty, universe;
};

const interval interval::empty = interval(+infinity_f32, -infinity_f32);
const interval interval::universe = interval(-infinity_f32, +infinity_f32);

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

struct HitRecord {
    point3 p;
    vec3 n;
    float t;
    bool isFrontFace;

    void setFaceNormal(const Ray &r, const vec3 &outwardNormal) {
        isFrontFace = glm::dot(r.getDir(), outwardNormal) < 0.0f;
        n = isFrontFace ? outwardNormal : -outwardNormal;
    }
};

class Model {
public:
    virtual ~Model() = default;
    virtual bool hit(const Ray &r, interval ray_t, HitRecord &rec) const = 0;
};

class Sphere : public Model {
public:
    Sphere(const point3 &center, float radius) : center(center), radius(std::fmax(0, radius)) {}

    bool hit(const Ray &r, interval ray_t, HitRecord &hitRecord) const override {
        vec3 oc = center - r.getOrigin();
        auto a = glm::dot(r.getDir(), r.getDir());
        auto h = glm::dot(r.getDir(), oc);
        auto c = glm::dot(oc, oc) - radius * radius;

        auto discriminant = h * h - a * c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.contains_open(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.contains_open(root)) return false;
        }

        hitRecord.t = root;
        hitRecord.p = r.at(hitRecord.t);
        vec3 outwardNormal = (hitRecord.p - center) / vec3(radius);
        hitRecord.setFaceNormal(r, outwardNormal);

        return true;
    }

private:
    point3 center;
    float radius;
};

class ModelList : public Model {
public:
    std::vector<shared_ptr<Model>> objects;

    ModelList() {}
    ModelList(shared_ptr<Model> object) { add(object); }

    void clear() { objects.clear(); }

    void add(shared_ptr<Model> object) {
        objects.push_back(object);
    }

    bool hit(const Ray &r, interval ray_t, HitRecord &rec) const override {
        HitRecord temp_rec;
        bool hitAnything = false;
        auto closest_so_far = ray_t.max;

        for (const auto &object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hitAnything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hitAnything;
    }
};

#endif // UTIL_H
