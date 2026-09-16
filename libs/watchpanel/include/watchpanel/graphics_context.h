#ifndef WATCHPANEL_GRAPHICS_CONTEXT_H_
#define WATCHPANEL_GRAPHICS_CONTEXT_H_

#include "graphics.h"
#include "raster.h"

#include <string>
#include <vector>

namespace watchpanel {

    // Owns all drawing/layout reasoning (glyph metrics, word-wrap, overflow
    // clipping) on top of a Raster, which only knows how to set pixels. This
    // is what Graphic subclasses actually call into, so every Raster backend
    // gets identical text layout for free.
    class GraphicsContext {
    public:

        explicit GraphicsContext(Raster *raster, const std::string &fontPath = "fonts/tom-thumb.bdf");

        void DrawText(
            const TextSpan *textSpan,
            const char *fontName,
            Color color,
            int x,
            int y,
            int width,
            int height,
            int letterSpacing,
            int lineOffset,
            Wrap wrap,
            Overflow overflow);

        void DrawRect(
            int x,
            int y,
            int width,
            int height,
            Color fill,
            Color stroke);

        // Real image decoding is a follow-up (see docs/README.md); for now
        // this draws a placeholder block, same as every Raster backend did
        // before this split.
        void DrawImage(
            int x,
            int y,
            int width,
            int height,
            const char *href);

    private:

        Raster *raster;
        std::string fontPath;

        struct ClipBox {
            bool active;
            int x, y, width, height;
        };

        void DrawGlyph(unsigned char ch, Color color, int x, int baselineY, const ClipBox &clip);
        int GlyphAdvance(unsigned char ch) const;
        std::vector<std::string> WrapWords(const std::string &text, int maxWidthPx, int letterSpacing) const;

    };

}

#endif // WATCHPANEL_GRAPHICS_CONTEXT_H_
