#ifndef WATCHPANEL_SVG_H_
#define WATCHPANEL_SVG_H_

#include "graphics.h"

#include "pugixml.hpp"

namespace watchpanel {

    class SvgCanvas : public Canvas {
    private:

        pugi::xml_document doc;
        pugi::xml_node root;

    public:

        SvgCanvas(int width, int height);
        virtual ~SvgCanvas();

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