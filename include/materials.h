#pragma once

#include "Constants.h"
#include "Util.h"

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

        if (cannot_refract || reflectance(cos_theta, ri) > randomFloat()) {
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