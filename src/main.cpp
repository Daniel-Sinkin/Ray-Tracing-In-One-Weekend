#include "Constants.h"
#include "Util.h"
#include "camera.h"

int main() {
    ModelList world;

    auto materialGround = make_shared<LambertianMaterial>(color(0.8f, 0.8f, 0.0f));
    auto materialCenter = make_shared<LambertianMaterial>(color(0.1f, 0.2f, 0.5f));
    auto materialLeft = make_shared<DialectricMaterial>(1.5f);
    auto materialBubble = make_shared<DialectricMaterial>(1.00 / 1.5f);
    auto materialRight = make_shared<MetalMaterial>(color(0.8f, 0.6f, 0.2f), 1.0f);

    world.add(make_shared<Sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f, materialGround));
    world.add(make_shared<Sphere>(point3(0.0f, 0.0f, -1.2f), 0.5f, materialCenter));
    world.add(make_shared<Sphere>(point3(-1.0f, 0.0f, -1.0f), 0.5f, materialLeft));
    world.add(make_shared<Sphere>(point3(-1.0f, 0.0f, -1.0f), 0.4f, materialBubble));
    world.add(make_shared<Sphere>(point3(1.0f, 0.0f, -1.0f), 0.5f, materialRight));

    Camera camera;

    camera.m_AspectRatio = 16.0f / 9.0f;
    camera.m_ImageWidth = 800;
    camera.m_SamplesPerPixel = 100;
    camera.m_MaxDepth = 50;

    camera.render(world);
}