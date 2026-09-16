#ifndef WATCHPANEL_SVG_RASTER_H_
#define WATCHPANEL_SVG_RASTER_H_

#include "watchpanel/raster.h"

#include "pugixml.hpp"

namespace watchpanel {

    // Renders a page as pixel-accurate SVG: each SetPixel call becomes a 1x1
    // <rect>, and the whole frame is scaled up so it displays as chunky,
    // crisp pixels instead of tiny native-resolution squares — the same
    // pixel grid GraphicsContext produces for LedRaster/TerminalRaster.
    class SvgRaster : public Raster {
    private:

        int width_;
        int height_;
        int pixelScale_;
        pugi::xml_document doc;
        pugi::xml_node root;
        pugi::xml_node scene;

        void Reset();

    public:

        SvgRaster(int width, int height, int pixelScale = 8);
        ~SvgRaster() override;

        int Width() const override { return width_; }
        int Height() const override { return height_; }
        void Clear() override;
        void SetPixel(int x, int y, Color color) override;

        void Save(const char *path);

    };

}

#endif // WATCHPANEL_SVG_RASTER_H_
