#include "Constants.h"
#include "Util.h"
#include "color.h"

DEF rayColor(const Ray &r, const ModelList &world) -> vec3 {
    HitRecord rec;
    if (world.hit(r, interval(0, infinity_f32), rec)) {
        return 0.5f * (rec.n + WHITE);
    }

    vec3 nu = r.getDir() / vec3(glm::length(r.getDir()));
    auto a = 0.5f * (nu.y + 1.0f);
    return (1 - a) * WHITE + a * color(0.5, 0.7, 1.0);
}

DEF main() -> int {
    auto aspect_ratio = 16.0f / 9.0f;
    size_t image_width = 800;

    // Calculate the image height, and ensure that it's at least 1.
    size_t image_height = (size_t)(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    ModelList world;
    world.add(make_shared<Sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<Sphere>(point3(0, -100.5, -1), 100));

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

            write_color(std::cout, rayColor(r, world));
        }
    }

    std::clog << "\rDone.\n";

    return EXIT_SUCCESS;
}