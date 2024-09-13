#ifndef UTIL_H
#define UTIL_H

#include "Constants.h"
#include "color.h"

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

void writeColor(std::ostream &out, const color &pixel_color) {
    float r = pixel_color.x;
    float g = pixel_color.y;
    float b = pixel_color.z;

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
    virtual bool hit(const Ray &r, Interval ray_t, HitRecord &rec) const = 0;
};

class Sphere : public Model {
public:
    Sphere(const point3 &center, float radius) : center(center), radius(std::fmax(0, radius)) {}

    bool hit(const Ray &r, Interval ray_t, HitRecord &hitRecord) const override {
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

class Camera {
public:
    float m_AspectRatio = 16.0f / 9.0f;
    int m_ImageWidth = 800;
    int m_SamplesPerPixel = 100;

    void render(const Model &world) {
        initialize();

        std::cout << "P3\n"
                  << m_ImageWidth << ' ' << m_ImageHeight << "\n255\n";

        for (int j = 0; j < m_ImageHeight; j++) {
            std::clog << "\rScanlines remaining: " << (m_ImageHeight - j) << ' ' << std::flush;
            for (int i = 0; i < m_ImageWidth; i++) {
                color pixelColor = BLACK;
                for (int sample = 0; sample < m_SamplesPerPixel; sample++) {
                    Ray r = getRay(i, j);
                    pixelColor += rayColor(r, world);
                }
                writeColor(std::cout, m_PixelSamplesScale * pixelColor);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int m_ImageHeight;
    float m_PixelSamplesScale;
    point3 m_Center;
    point3 m_PixelOrigin; // pixel00_loc
    vec3 m_PixelDeltaU;
    vec3 m_PixelDeltaV;

    void initialize() {
        m_ImageHeight = int(m_ImageWidth / m_AspectRatio);
        m_ImageHeight = (m_ImageHeight < 1) ? 1 : m_ImageHeight;

        m_PixelSamplesScale = 1.0f / m_SamplesPerPixel;

        m_Center = ORIGIN;

        float focal_length = 1.0f;
        float viewport_height = 2.0f;
        float viewport_width = viewport_height * (float(m_ImageWidth) / m_ImageHeight);

        vec3 viewportU = vec3(viewport_width, 0, 0);
        vec3 viewportV = vec3(0, -viewport_height, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        m_PixelDeltaU = viewportU / vec3((float)m_ImageWidth);
        m_PixelDeltaV = viewportV / vec3((float)m_ImageHeight);

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = m_Center - vec3(0.0f, 0.0f, focal_length) - viewportU / vec3(2.0f) - viewportV / vec3(2.0f);
        m_PixelOrigin = viewport_upper_left + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);
    }

    Ray getRay(int i, int j) {
        auto offset = sampleSquare();
        auto pixelSample = m_PixelOrigin + (i + offset.x) * m_PixelDeltaU + (j + offset.y) * m_PixelDeltaV;

        auto rayOrigin = m_Center;
        auto rayDirection = pixelSample - rayOrigin;

        return Ray(rayOrigin, rayDirection);
    }

    vec3 sampleSquare() const {
        return vec3(random_float() - 0.5f, random_float() - 0.5f, 0.0f);
    }

    color rayColor(const Ray &r, const Model &world) const {
        HitRecord rec;
        if (world.hit(r, Interval(0, infinity_f32), rec)) {
            return 0.5f * (rec.n + WHITE);
        }

        vec3 nu = glm::normalize(r.getDir());

        auto a = 0.5f * (nu.y + 1.0f);
        return (1.0f - a) * WHITE + a * color(0.5f, 0.7f, 1.0f);
    }
};

#endif // UTIL_H
