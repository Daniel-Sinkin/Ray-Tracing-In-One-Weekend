#include "Util.h"
#include "camera.h"

struct Example {
    Camera camera;
    ModelList world;

    // Added constructor to initialize camera and world
    Example(const Camera &cam, const ModelList &w) : camera(cam), world(w) {}

    DEF run() -> void {
        camera.render(world);
    }
};

DEF Example_Final() -> Example {
    ModelList world;

    auto ground_material = make_shared<LambertianMaterial>(color(0.5, 0.5, 0.5));
    world.add(make_shared<Sphere>(point3(0.0f, -1000.0f, 0.0f), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = randomFloat();
            point3 center(a + 0.9f * randomFloat(), 0.2f, b + 0.9f * randomFloat());

            if ((center - point3(4.0f, 0.2f, 0.0f)).length() > 0.9f) {
                shared_ptr<Material> sphereMaterial;

                if (choose_mat < 0.8f) {
                    // diffuse
                    auto albedo = random_vec3() * random_vec3();
                    sphereMaterial = make_shared<LambertianMaterial>(albedo);
                    world.add(make_shared<Sphere>(center, 0.2f, sphereMaterial));
                } else if (choose_mat < 0.95f) {
                    // metal
                    auto albedo = random_vec3(0.5f, 1.0f);
                    auto fuzz = randomFloat(0.0f, 0.5f);
                    sphereMaterial = make_shared<MetalMaterial>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2f, sphereMaterial));
                } else {
                    // glass
                    sphereMaterial = make_shared<DialectricMaterial>(1.5f);
                    world.add(make_shared<Sphere>(center, 0.2f, sphereMaterial));
                }
            }
        }
    }

    auto material1 = make_shared<DialectricMaterial>(1.5);
    world.add(make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<LambertianMaterial>(color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<MetalMaterial>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(point3(4, 1, 0), 1.0, material3));

    Camera camera;

    camera.m_AspectRatio = 16.0f / 9.0f;
    camera.m_ImageWidth = 800;
    camera.m_SamplesPerPixel = 10;
    camera.m_MaxDepth = 50;

    camera.m_VFOV = 20.0f;
    camera.m_Eye = point3(13.0f, 2.0f, 3.0f);
    camera.m_Center = point3(0.0f, 0.0f, 0.0f);
    camera.m_Up = vec3(0.0f, 1.0f, 0.0f);

    camera.m_DOFAngle = 0.6f;
    camera.m_FocusDistance = 10.0f;

    return Example(camera, world);
}

DEF Example_Materials(float vfov = 90.0f) -> Example {
    ModelList world;

    const auto material_ground = make_shared<LambertianMaterial>(color(0.8f, 0.8f, 0.0f));
    const auto material_center = make_shared<LambertianMaterial>(color(0.1f, 0.2f, 0.5f));
    const auto material_left = make_shared<DialectricMaterial>(1.50f);
    const auto material_bubble = make_shared<DialectricMaterial>(1.00f / 1.50f);
    const auto material_right = make_shared<MetalMaterial>(color(0.8f, 0.6f, 0.2f), 1.0f);

    world.add(make_shared<Sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f, material_ground));
    world.add(make_shared<Sphere>(point3(0.0f, 0.0f, -1.2f), 0.5f, material_center));
    world.add(make_shared<Sphere>(point3(-1.0f, 0.0f, -1.0f), 0.5f, material_left));
    world.add(make_shared<Sphere>(point3(-1.0f, 0.0f, -1.0f), 0.4f, material_bubble));
    world.add(make_shared<Sphere>(point3(1.0f, 0.0f, -1.0f), 0.5f, material_right));

    Camera cam;

    cam.m_AspectRatio = 16.0f / 9.0f;
    cam.m_ImageWidth = 800;
    cam.m_SamplesPerPixel = 100;
    cam.m_MaxDepth = 50;

    cam.m_VFOV = vfov;
    cam.m_Eye = point3(-2.0f, 2.0f, 1.0f);
    cam.m_Center = point3(0.0f, 0.0f, -1.0f);
    cam.m_Up = vec3(0.0f, 1.0f, 0.0f);

    cam.m_DOFAngle = 10.0f;
    cam.m_FocusDistance = 3.4f;

    return Example(cam, world);
}

DEF Example_FOV() -> Example {
    ModelList world;

    const auto R = std::cos(pi / 4.0f);

    const auto material_left = make_shared<LambertianMaterial>(color(0.0f, 0.0f, 1.0f));
    const auto material_right = make_shared<LambertianMaterial>(color(1.0f, 0.0f, 0.0f));

    world.add(make_shared<Sphere>(point3(-R, 0.0f, -1.0f), R, material_left));
    world.add(make_shared<Sphere>(point3(R, 0.0f, -1.0f), R, material_right));

    Camera cam;

    cam.m_AspectRatio = 16.0f / 9.0f;
    cam.m_ImageWidth = 800;
    cam.m_SamplesPerPixel = 100;
    cam.m_MaxDepth = 50;

    cam.m_VFOV = 90.0f;

    // Return the Example with the camera and world properly initialized
    return Example(cam, world);
}
