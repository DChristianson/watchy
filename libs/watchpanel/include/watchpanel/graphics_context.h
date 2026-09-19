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

        explicit GraphicsContext(Raster *raster, const std::string &fontPath = "fonts/tom-thumb.bdf",
                                  const std::string &cacheDir = "cache");

        // Returns the total pixel height of the laid-out text (line count *
        // line spacing), regardless of how much of it actually fit in the
        // box -- callers (e.g. FlipGraphic) use this to detect when content
        // exceeds its box and how far it still needs to scroll.
        //
        // scrollOffsetY shifts the whole block up by that many pixels
        // before drawing (e.g. for a vertical auto-scroll effect); passing
        // it forces box-clipping regardless of `overflow`, since scrolled
        // content leaving artifacts outside the box would look broken.
        int DrawText(
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
            Overflow overflow,
            int scrollOffsetY = 0);

        void DrawRect(
            int x,
            int y,
            int width,
            int height,
            Color fill,
            Color stroke);

        // Local files are decoded and nearest-neighbor scaled into the
        // box; remote http(s):// URLs are fetched through hamper's cache
        // first (maxAgeSeconds controls how stale a cached copy may be
        // before a refresh is attempted -- see watchpanel/hamper.h for
        // the "stale data still wins over no data" fallback semantics).
        // Falls back to a placeholder block if decoding/fetching fails.
        void DrawImage(
            int x,
            int y,
            int width,
            int height,
            const char *href,
            long maxAgeSeconds = 24 * 60 * 60);

        // Direct single-pixel write, bypassing all layout reasoning --
        // e.g. FadeTransitionGraphic uses this to composite two off-screen
        // renders into this context's raster. Kept on GraphicsContext
        // (rather than exposing the raw Raster*) so it stays the only
        // thing any Graphic ever touches.
        void SetPixel(int x, int y, Color color);

    private:

        Raster *raster;
        std::string fontPath;
        std::string cacheDir;

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
