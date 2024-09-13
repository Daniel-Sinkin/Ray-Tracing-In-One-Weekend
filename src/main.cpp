#include "Constants.h"
#include "Util.h"

int main() {
    ModelList world;

    world.add(make_shared<Sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f));
    world.add(make_shared<Sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f));

    Camera camera;

    camera.m_AspectRatio = 16.0f / 9.0f;
    camera.m_ImageWidth = 800;

    camera.render(world);
}