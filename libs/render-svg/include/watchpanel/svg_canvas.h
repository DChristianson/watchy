#ifndef WATCHPANEL_SVG_H_
#define WATCHPANEL_SVG_H_

#include "graphics.h"

#include "pugixml.hpp"

#include <string>

namespace watchpanel {

    // Renders a page as pixel-accurate SVG: text is rasterized glyph-by-glyph
    // from the same BDF font LedCanvas/TerminalCanvas use (one <rect> per lit
    // pixel) rather than relying on the browser's own font rendering, then
    // the whole frame is scaled up so it displays as chunky, crisp pixels
    // instead of tiny native-resolution squares.
    class SvgCanvas : public Canvas {
    private:

        int width;
        int height;
        int pixelScale;
        std::string fontPath;
        pugi::xml_document doc;
        pugi::xml_node root;
        pugi::xml_node scene;

        void Reset();
        void DrawGlyph(unsigned char ch, const Color &color, int x, int y);

    public:

        SvgCanvas(int width, int height, int pixelScale = 8,
                  const std::string &fontPath = "fonts/tom-thumb.bdf");
        virtual ~SvgCanvas();

        // Discards everything drawn so far, so the canvas can be reused for
        // the next frame in a render loop.
        void Clear();

        void DrawText(
            const TextSpan *textSpan,
            const char *fontName,
            Color color,
            int x,
            int y,
            int letterSpacing,
            int lineOffset);

        void DrawRect(
            int x,
            int y,
            int width,
            int height,
            Color fill,
            Color stroke);

        void DrawImage(
            int x,
            int y,
            int width,
            int height,
            const char *href);

        void Save(const char *path);

    };

}

#endif // WATCHPANEL_SVG_H_
