#pragma once

#include "Constants.h"
#include "Util.h"
#include "materials.h"
#include "models.h"

class Camera {
public:
    float m_AspectRatio = 16.0f / 9.0f;
    int m_ImageWidth = 100;
    int m_SamplesPerPixel = 10;
    int m_MaxDepth = 10;

    float m_VFOV = 20.0f;
    point3 m_Eye = ORIGIN;
    point3 m_Center = CAMERA_FORWARD;
    vec3 m_Up = WORLDSPACE_UP;

    float m_DOFAngle = 0.0f;
    float m_FocusDistance = 0.0f;

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
                    pixelColor += this->rayColor(r, m_MaxDepth, world);
                }
                writeColor(std::cout, m_PixelSamplesScale * pixelColor);
            }

            // End timing the scanline
            auto endTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> scanlineDuration = endTime - startTime;

            // Estimate remaining time
            int remainingScanlines = m_ImageHeight - (j + 1); // Subtract 1 because j is 0-based
            float estimatedRemainingTime = scanlineDuration.count() * remainingScanlines;

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
    point3 m_PixelOrigin; // pixel00_loc
    vec3 m_PixelDeltaU;
    vec3 m_PixelDeltaV;
    vec3 m_U, m_V, m_W;

    vec3 m_DOFDiskU; // Horizontal radius
    vec3 m_DOFDiskV; // Vertical Radius

    void initialize() {
        m_ImageHeight = int(m_ImageWidth / m_AspectRatio);
        m_ImageHeight = (m_ImageHeight < 1) ? 1 : m_ImageHeight;

        m_PixelSamplesScale = 1.0f / m_SamplesPerPixel;

        vec3 lookDirection = m_Eye - m_Center;
        float theta = degrees_to_radians(m_VFOV);
        float h = std::tan(theta / 2.0f);
        float viewport_height = 2.0f * h * m_FocusDistance;
        float viewport_width = viewport_height * (float(m_ImageWidth) / m_ImageHeight);

        m_W = glm::normalize(lookDirection);
        m_U = glm::normalize(glm::cross(m_Up, m_W));
        m_V = glm::cross(m_W, m_U);

        vec3 viewportU = vec3(viewport_width) * m_U;
        vec3 viewportV = vec3(viewport_height) * (-m_V);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        m_PixelDeltaU = viewportU / vec3((float)m_ImageWidth);
        m_PixelDeltaV = viewportV / vec3((float)m_ImageHeight);

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = m_Eye - (vec3(m_FocusDistance) * m_W) - viewportU / vec3(2.0f) - viewportV / vec3(2.0f);
        m_PixelOrigin = viewport_upper_left + 0.5f * (m_PixelDeltaU + m_PixelDeltaV);

        auto DOFRadius = m_FocusDistance * std::tan(degrees_to_radians(m_DOFAngle / 2.0f));
        m_DOFDiskU = m_U * vec3(DOFRadius);
        m_DOFDiskV = m_V * vec3(DOFRadius);
    }

    Ray getRay(int i, int j) {
        auto offset = this->sampleSquare();
        auto pixelSample = m_PixelOrigin + (i + offset.x) * m_PixelDeltaU + (j + offset.y) * m_PixelDeltaV;

        auto rayOrigin = (m_DOFAngle <= 0.0f) ? m_Eye : this->sampleDOFDisk();
        auto rayDirection = pixelSample - rayOrigin;

        return Ray(rayOrigin, rayDirection);
    }

    vec3 sampleSquare() const {
        return vec3(randomFloat() - 0.5f, randomFloat() - 0.5f, 0.0f);
    }

    point3 sampleDOFDisk() const {
        point3 p = randomInUnitDisk();
        return m_Eye + (p.x * m_DOFDiskU) + (p.y * m_DOFDiskV);
    }

    color rayColor(const Ray &r, int depth, const Model &world) const {
        if (depth <= 0) return BLACK;

        HitRecord hitRecord;
        if (world.hit(r, Interval(SHADOW_ACNE_FIX_THRESHOLD, infinity_f32), hitRecord)) {
            Ray scattered;
            color attenuation;
            if (hitRecord.material->scatter(r, hitRecord, attenuation, scattered)) {
                return attenuation * this->rayColor(scattered, depth - 1, world);
            }
            return BLACK;
        }

        vec3 nu = glm::normalize(r.getDir());
        float a = 0.9f * (nu.y + 1.0f);
        return (1.0f - a) * WHITE + a * color(0.5f, 0.7f, 1.0f);
    }
};