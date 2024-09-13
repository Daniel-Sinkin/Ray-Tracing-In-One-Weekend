#ifndef UTIL_H
#define UTIL_H

#include "Constants.h"
#include "color.h"

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

class Camera {
public:
    float m_AspectRatio = 1.0;
    int m_ImageWidth = 100;

    void render(const Model &world) {
        initialize();

        std::cout << "P3\n"
                  << image_width << ' ' << mImageHeight << "\n255\n";

        for (int j = 0; j < mImageHeight; j++) {
            std::clog << "\rScanlines remaining: " << (mImageHeight - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                auto pixel_center = m_PixelOrigin + (vec3((float)i) * m_PixelDeltaU) + (vec3((float)j) * m_PixelDeltaV);
                auto ray_direction = pixel_center - m_Center;
                Ray r(m_Center, ray_direction);

                color pixel_color = ray_color(r, world);
                write_color(std::cout, pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int mImageHeight;
    point3 m_Center;
    point3 m_PixelOrigin; // pixel00_loc
    vec3 m_PixelDeltaU;
    vec3 m_PixelDeltaV;

    void initialize() {
        mImageHeight = int(image_width / m_AspectRatio);
        mImageHeight = (mImageHeight < 1) ? 1 : mImageHeight;

        m_Center = point3(0, 0, 0);

        // Determine viewport dimensions.
        float focal_length = 1.0f;
        float viewport_height = 2.0f;
        float viewport_width = viewport_height * (float(image_width) / mImageHeight);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewportU = vec3(viewport_width, 0, 0);
        vec3 viewportV = vec3(0, -viewport_height, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        m_PixelDeltaU = viewportU / vec3((float)image_width);
        m_PixelDeltaV = viewportV / vec3((float)mImageHeight);

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = m_Center - vec3(0.0f, 0.0f, focal_length) - viewportU / vec3(2.0f) - viewportV / vec3(2.0f);
        m_PixelOrigin = viewport_upper_left + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);
    }

    color ray_color(const Ray &r, const Model &world) const {
        HitRecord rec;
        if (world.hit(r, interval(0, infinity_f32), rec)) {
            return 0.5f * (rec.n + WHITE);
        }

        vec3 nu = glm::normalize(r.getDir());

        auto a = 0.5f * (nu.y + 1.0f);

        return (1.0f - a) * WHITE + a * color(0.5f, 0.7f, 1.0f);
    }
};

#endif // UTIL_H
