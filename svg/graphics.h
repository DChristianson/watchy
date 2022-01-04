
#ifndef WATCHPANEL_GRAPHICS_H_
#define WATCHPANEL_GRAPHICS_H_

#include "strings.h"

#include <stdint.h>

namespace watchpanel {

    struct Color {

        Color() : r(0), g(0), b(0) {}
        Color(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}

        uint8_t r;
        uint8_t g;
        uint8_t b;

        void Format(std::string &out);

        static Color Parse(const char * colorName);

    };

    struct TextSpan {

        TextSpan(const char *text) : text(text), nextSpan(NULL) {}
        std::string text;
        TextSpan* nextSpan;

    };

    class Canvas {
    public:
    
        virtual ~Canvas();

        virtual void DrawText(
            const TextSpan *textSpan,
            const char *fontName,
            Color color,
            int x,
            int y,
            int letterSpacing,
            int lineOffset);

        virtual void DrawRect(
            int x,
            int y,
            int width,
            int height,
            Color fill,
            Color stroke);

        virtual void DrawImage(
            int x,
            int y,
            int width,
            int height,
            const char *href        
        );

    };

    class Graphic {
    protected:

        Canvas * canvas;
        Graphic(Canvas * canvas) : canvas(canvas) {}

    public:

        virtual ~Graphic();
        virtual void Draw() {}

    };

    class TextGraphic : public Graphic {
    private:

        TextSpan *firstSpan;
        TextSpan *lastSpan;
        std::string fontName;
        Color color;
        int x;
        int y;
        int letterSpacing;
        int lineOffset;

    public:

        TextGraphic(
            Canvas * canvas,
            const char *fontName,
            Color color,
            int x,
            int y,
            int letterSpacing,
            int lineOffset);
        ~TextGraphic();

        TextSpan &AppendText(const char *text);

        void Draw();

    };

    class RectGraphic: public Graphic {
    private:

        int x;
        int y;
        int width;
        int height;
        Color fill;
        Color stroke;

    public:

        RectGraphic(
            Canvas * canvas,
            int x,
            int y,
            int width,
            int height,
            Color fill,
            Color stroke);
        ~RectGraphic();

        void Draw();

    };

    class ImageGraphic : public Graphic {
    private:

        int x;
        int y;
        int width;
        int height;
        std::string href;

    public:

        ImageGraphic(
            Canvas * canvas,
            int x,
            int y,
            int width,
            int height,
            const char *href
        );
        ~ImageGraphic();

        void SetHRef(const char *href);
        void Draw();

    };

}

#endif // WATCHPANEL_GRAPHICS_H_