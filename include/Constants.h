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

using point3 = glm::vec3;
using color = glm::vec3;

#define DEF auto
using glm::vec3;
using std::cout;
using std::make_shared;
using std::optional;
using std::shared_ptr;

constexpr size_t image_width = 256;
constexpr size_t image_height = 256;

constexpr vec3 CAMERA_FORWARD = vec3(0.0f, 0.0f, -1.0f);

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

struct hit_record {
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
    virtual bool hit(const Ray &r, float tMin, float tMax, hit_record &rec) const = 0;
};

class sphere : public Model {
public:
    sphere(const point3 &center, double radius) : center(center), radius(std::fmax(0, radius)) {}

    bool hit(const Ray &r, float tMin, float tMax, hit_record &hitRecord) const override {
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
        if (root <= tMin || tMax <= root) {
            root = (h + sqrtd) / a;
            if (root <= tMin || tMax <= root)
                return false;
        }

        hitRecord.t = root;
        hitRecord.p = r.at(hitRecord.t);
        vec3 outwardNormal = (hitRecord.p - center) / vec3(radius);
        hitRecord.setFaceNormal(r, outwardNormal);

        return true;
    }

    class ModelList : public Model {
    public:
        std::vector<shared_ptr<Model>> objects;

        ModelList() {}
        ModelList(shared_ptr<Model> object) { add(object); }

        void clear() { objects.clear(); }

        void add(shared_ptr<Model> object) {
            objects.push_back(object);
        }

        bool hit(const Ray &r, float tMin, float tMax, hit_record &rec) const override {
            hit_record temp_rec;
            bool hitAnything = false;
            auto closest_so_far = tMax;

            for (const auto &object : objects) {
                if (object->hit(r, tMin, closest_so_far, temp_rec)) {
                    hitAnything = true;
                    closest_so_far = temp_rec.t;
                    rec = temp_rec;
                }
            }

            return hitAnything;
        }
    };

private:
    point3 center;
    double radius;
};

#endif // CONSTANTS_H
