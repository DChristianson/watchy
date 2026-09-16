#ifndef WATCHPANEL_RASTER_H_
#define WATCHPANEL_RASTER_H_

#include "graphics.h"

namespace watchpanel {

    // The minimal contract a display backend must implement. All layout
    // reasoning (text wrapping, overflow, glyph metrics) lives in
    // GraphicsContext, which is the only thing that calls into this
    // interface — a Raster just knows how to set one pixel.
    class Raster {
    public:

        virtual ~Raster();

        virtual int Width() const = 0;
        virtual int Height() const = 0;
        virtual void Clear() = 0;
        virtual void SetPixel(int x, int y, Color color) = 0;

    };

}

#endif // WATCHPANEL_RASTER_H_
