
#include <stdint.h>

namespace rgbmatrix {

    class Canvas;
    class Font;
    struct Color {
        Color() : r(0), g(0), b(0) {}
        Color(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

}