#include "Constants.h"
#include "color.h"

DEF hit_sphere(const point3 &center, float radius, const Ray &r) -> optional<float> {
    vec3 oc = center - r.getOrigin();
    float a = glm::dot(r.getDir(), r.getDir());
    float h = glm::dot(r.getDir(), oc);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = h * h - a * c;
    if (discriminant < 0) return std::nullopt;

    return (h - std::sqrt(discriminant)) / a;
}

DEF ray_color(const Ray &r) -> vec3 {
    auto t = hit_sphere(point3(0.0f, 0.0f, -1.0f), 0.5f, r);
    if (t && *t > 0.0f) {
        vec3 N = glm::normalize(r.at(*t) - CAMERA_FORWARD);
        auto retval = 0.5f * color(N.x + 1.0f, N.y + 1.0f, N.z + 1.0f);

        return retval;
    }

    vec3 direction_n = r.getDir() / glm::length(r.getDir());
    float lambda = 0.5f * float(direction_n.y + 1.0f);
    return vec3(1.0f - lambda) * vec3(1.0f, 1.0f, 1.0f) + vec3(lambda) * vec3(0.5f, 0.7f, 1.0f);
}

DEF main() -> int {
    auto aspect_ratio = 16.0f / 9.0f;
    size_t image_width = 800;

    // Calculate the image height, and ensure that it's at least 1.
    size_t image_height = (size_t)(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    auto focal_length = 1.0f;
    auto viewport_height = 2.0f;
    auto viewport_width = viewport_height * (float(image_width) / image_height);
    auto camera_center = point3(0.0f, 0.0f, 0.0f);

    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    auto pixel_delta_u = viewport_u * (float)(1.0f / image_width);
    auto pixel_delta_v = viewport_v * (float)(1.0f / image_height);

    auto viewport_upper_left = camera_center - vec3(0.0f, 0.0f, focal_length) - viewport_u / 2.0f - viewport_v / 2.0f;
    auto pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

    std::cout << "P3\n"
              << image_width << ' ' << image_height << "\n255\n";

    for (size_t j = 0; j < image_height; ++j) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (size_t i = 0; i < image_width; i++) {
            auto pixel_center = pixel00_loc + ((float)i * pixel_delta_u) + ((float)j * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            Ray r(camera_center, ray_direction);

            vec3 pixel_color = ray_color(r);
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.\n";

    return EXIT_SUCCESS;
}