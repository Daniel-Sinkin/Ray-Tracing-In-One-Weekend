#ifndef UTIL_H
#define UTIL_H

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
class Material {
public:
    virtual ~Material() = default;

    virtual bool scatter(const Ray &rIn, const HitRecord &hitRecord, color &attenuation, Ray &scattered) const { return false; }
};

class LambertianMaterial : public Material {
public:
    LambertianMaterial(const color &albedo) : albedo(albedo) {}

    bool scatter(const Ray &rIn, const HitRecord &hitRecord, color &attenuation, Ray &scattered) const override {
        auto scatterDirection = hitRecord.n + random_vec3_n();
        if (scatterDirection.length() < 1e-8) scatterDirection = hitRecord.n;

        scattered = Ray(hitRecord.p, scatterDirection);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

class MetalMaterial : public Material {
public:
    MetalMaterial(const color &albedo, float fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const Ray &rIn, const HitRecord &hitRecord, color &attenuation, Ray &scattered) const override {
        vec3 reflected = reflect(rIn.getDir(), hitRecord.n);
        reflected = glm::normalize(reflected) + (fuzz * random_vec3_n());
        scattered = Ray(hitRecord.p, reflected);
        attenuation = albedo;
        return (glm::dot(scattered.getDir(), hitRecord.n) > 0);
    }

private:
    color albedo;
    float fuzz;
};

class DialectricMaterial : public Material {
public:
    DialectricMaterial(float refraction_index) : refractionIndexBase(refraction_index) {}

    bool scatter(const Ray &r_in, const HitRecord &hitRecord, color &attenuation, Ray &scattered) const override {
        attenuation = WHITE;
        float ri = hitRecord.isFrontFace ? (1.0f / refractionIndexBase) : refractionIndexBase;

        vec3 unit_direction = glm::normalize(r_in.getDir());
        float cos_theta = std::fmin(glm::dot(-unit_direction, hitRecord.n), 1.0f);
        float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0f;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_float()) {
            direction = reflect(unit_direction, hitRecord.n);
        } else {
            direction = refract(unit_direction, hitRecord.n, ri);
        }

        scattered = Ray(hitRecord.p, direction);
        return true;
    }

private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    float refractionIndexBase;

    static float reflectance(float cosine, float refractionIndex) {
        float r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow(1.0f - cosine, 5);
    }
};

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

class Camera {
public:
    float m_AspectRatio = 16.0f / 9.0f;
    int m_ImageWidth = 800;
    int m_SamplesPerPixel = 100;
    int m_MaxDepth = 50;

    void render(const Model &world) {
        initialize();

        // Start total timer
        auto totalStartTime = std::chrono::high_resolution_clock::now();

        std::cout << "P3\n"
                  << m_ImageWidth << ' ' << m_ImageHeight << "\n255\n";

        for (int j = 0; j < m_ImageHeight; j++) {
            // Start timing the scanline
            auto startTime = std::chrono::high_resolution_clock::now();

            std::clog << "\rScanlines remaining: " << (m_ImageHeight - j) << ' ' << std::flush;

            for (int i = 0; i < m_ImageWidth; i++) {
                color pixelColor = BLACK;
                for (int sample = 0; sample < m_SamplesPerPixel; sample++) {
                    Ray r = getRay(i, j);
                    pixelColor += rayColor(r, m_MaxDepth, world);
                }
                writeColor(std::cout, m_PixelSamplesScale * pixelColor);
            }

            // End timing the scanline
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> scanlineDuration = endTime - startTime;

            // Estimate remaining time
            int remainingScanlines = m_ImageHeight - (j + 1); // Subtract 1 because j is 0-based
            double estimatedRemainingTime = scanlineDuration.count() * remainingScanlines;

            // Print the time taken for the previous scanline and estimate remaining time
            std::clog << " (Previous scanline took " << scanlineDuration.count() << " seconds)"
                      << " | Estimated time remaining: " << estimatedRemainingTime << " seconds." << std::flush;
        }

        // End total timer and calculate the total duration
        auto totalEndTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> totalRenderDuration = totalEndTime - totalStartTime;

        std::clog << std::flush << "\rDone. Total render time: " << totalRenderDuration.count() << " seconds.\n";
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

    color rayColor(const Ray &r, int depth, const Model &world) const {
        if (depth <= 0) return BLACK;

        HitRecord hitRecord;
        if (world.hit(r, Interval(SHADOW_ACNE_FIX_THRESHOLD, infinity_f32), hitRecord)) {
            Ray scattered;
            color attenuation;
            if (hitRecord.material->scatter(r, hitRecord, attenuation, scattered)) {
                return attenuation * rayColor(scattered, depth - 1, world);
            }
            return BLACK;
        }

        vec3 nu = glm::normalize(r.getDir());
        float a = 0.9f * (nu.y + 1.0f);
        return (1.0f - a) * WHITE + a * color(0.5f, 0.7f, 1.0f);
    }
};

#endif // UTIL_H