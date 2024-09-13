#ifndef COLOR_H
#define COLOR_H

#include "Constants.h"

constexpr auto WHITE = color(1.0f);
constexpr auto RED = color(1.0f, 0.0f, 0.0f);
constexpr auto GREEN = color(0.0f, 1.0f, 0.0f);
constexpr auto BLUE = color(0.0f, 0.0f, 1.0f);
constexpr auto BLACK = color(0.0f);

void write_color(std::ostream &out, const color &pixel_color) {
    auto r = pixel_color.x;
    auto g = pixel_color.y;
    auto b = pixel_color.z;

    // Translate the [0,1] component values to the byte range [0,255].
    int rbyte = int(255.999 * r);
    int gbyte = int(255.999 * g);
    int bbyte = int(255.999 * b);

    // Write out the pixel color components.
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif // COLOR_H