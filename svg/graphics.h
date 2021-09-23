
#ifndef WATCHPANEL_GRAPHICS_H_
#define WATCHPANEL_GRAPHICS_H_

#include "strings.h"

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

namespace watchpanel {

    class Graphic {
    public:

        virtual ~Graphic();
        
        virtual void Draw(rgbmatrix::Canvas * canvas) {}

    };

    class TextGraphic : public Graphic {
    private:

        std::string value;
        const rgbmatrix::Font * font;
        rgbmatrix::Color color;
        int x;
        int y;
        int letter_spacing;
        int line_offset;

    public:

        TextGraphic(
            const char * text,
            const rgbmatrix::Font * font,
            rgbmatrix::Color color,
            int x,
            int y,
            int letter_spacing,
            int line_offset);
        ~TextGraphic();

        void SetValue(const char *str) { value = str; }

        void Draw(rgbmatrix::Canvas * canvas);

    };

    class RectGraphic: public Graphic {
    public:

        RectGraphic();
        ~RectGraphic();

        void Draw(rgbmatrix::Canvas * canvas);

    };

    class ImageGraphic : public Graphic {
    public:

        ImageGraphic();
        ~ImageGraphic();

        void Draw(rgbmatrix::Canvas * canvas);

    };

}

#endif // WATCHPANEL_GRAPHICS_H_