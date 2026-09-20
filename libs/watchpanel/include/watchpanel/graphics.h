
#ifndef WATCHPANEL_GRAPHICS_H_
#define WATCHPANEL_GRAPHICS_H_

#include "strings.h"
#include "update.h"

#include <stdint.h>
#include <vector>

namespace watchpanel {

    class GraphicsContext;

    struct Color {

        Color() : r(0), g(0), b(0) {}
        Color(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}

        uint8_t r;
        uint8_t g;
        uint8_t b;

        void Format(std::string &out) const;

        static Color Parse(const char * colorName);

        bool operator==(const Color &other) const {
            return r == other.r && g == other.g && b == other.b;
        }
        bool operator!=(const Color &other) const {
            return !(*this == other);
        }

    };

    struct TextSpan {

        TextSpan(const char *text) : text(text), nextSpan(NULL) {}
        std::string text;
        TextSpan* nextSpan;

    };

    enum class Wrap {
        kNone,
        kWord,
    };

    enum class Overflow {
        kVisible,
        kClip,
    };

    class Graphic {
    protected:

        GraphicsContext * context;
        Graphic(GraphicsContext * context) : context(context) {}

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
        int width;
        int height;
        int letterSpacing;
        int lineOffset;
        Wrap wrap;
        Overflow overflow;
        int scrollOffsetY;
        int lastContentHeight;

    public:

        TextGraphic(
            GraphicsContext * context,
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
        ~TextGraphic();

        TextSpan &AppendText(const char *text);

        // Shifts the drawn text up by this many pixels (see
        // GraphicsContext::DrawText); used by FlipGraphic to implement
        // vertical auto-scroll for content taller than its box.
        void SetScrollOffset(int offsetY) { scrollOffsetY = offsetY; }

        // Total pixel height of the text as laid out on the most recent
        // Draw() call (0 before the first Draw()) -- lets a caller detect
        // whether content exceeds its box.
        int LastContentHeight() const { return lastContentHeight; }

        void Draw();

    };

    // Draws a fixed set of child graphics in sequence -- e.g. wrapping an
    // entire page's display list as one Graphic so FadeTransitionGraphic
    // can treat it as a single "to" side to fade into. Owns its children.
    class GroupGraphic : public Graphic {
    private:

        std::vector<Graphic *> children;

    public:

        GroupGraphic(GraphicsContext * context);
        ~GroupGraphic();

        void AddChild(Graphic *child);
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
            GraphicsContext * context,
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
        long maxAgeSeconds;

    public:

        ImageGraphic(
            GraphicsContext * context,
            int x,
            int y,
            int width,
            int height,
            const char *href,
            long maxAgeSeconds = 24 * 60 * 60
        );
        ~ImageGraphic();

        void SetHRef(const char *href);
        void Draw();

    };

    // A text box that cycles through the elements of a JSON array over
    // time (see the itemsPath/period constructor args), re-scoping its
    // children's relative template paths to the current item each time it
    // advances. Wraps a TextGraphic by composition for the actual layout/
    // rendering, and is itself an Updateable so WatchPage can drive its
    // timing the same way everything else gets (now, deltaSeconds).
    class FlipGraphic : public Graphic, public Updateable {
    private:

        TextGraphic *inner;
        std::string itemsPath;
        long periodSeconds;
        int currentIndex;
        long lastFlipTime;
        bool hasFlippedOnce;
        int height;
        int scrollSpeedPxPerSec;
        int scrollOffsetPx;
        std::vector<Updateable *> childUpdates;

    public:

        FlipGraphic(
            GraphicsContext * context,
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
            const char *itemsPath,
            long periodSeconds,
            int scrollSpeedPxPerSec = 0
        );
        ~FlipGraphic();

        TextSpan &AppendText(const char *text);
        void AddChildUpdate(Updateable *update);

        void Update(const Model &model, long now, long deltaSeconds);
        void Draw();

    };

}

#endif // WATCHPANEL_GRAPHICS_H_
