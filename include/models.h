#pragma once

#include "Constants.h"
#include "Util.h"

class Model {
public:
    virtual ~Model() = default;
    virtual bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const = 0;
};

class Sphere : public Model {
public:
    Sphere(const point3 &center, float radius, shared_ptr<Material> material) : m_Center(center), m_Radius(std::fmax(0, radius)), m_Material(material) {}

    bool hit(const Ray &r, Interval ray_t, HitRecord &hitRecord) const override {
        vec3 oc = m_Center - r.getOrigin();
        auto a = glm::dot(r.getDir(), r.getDir());
        auto h = glm::dot(r.getDir(), oc);
        auto c = glm::dot(oc, oc) - m_Radius * m_Radius;

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
        vec3 outwardNormal = (hitRecord.p - m_Center) / vec3(m_Radius);
        hitRecord.setFaceNormal(r, outwardNormal);
        hitRecord.material = m_Material;

        return true;
    }

private:
    point3 m_Center;
    float m_Radius;
    shared_ptr<Material> m_Material;
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

    bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const override {
        HitRecord temp_rec;
        bool hitAnything = false;
        auto closest_so_far = ray_t.m_Max;

        for (const auto &object : objects) {
            if (object->hit(r, Interval(ray_t.m_Min, closest_so_far), temp_rec)) {
                hitAnything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hitAnything;
    }
};
