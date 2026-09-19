#ifndef WATCHPANEL_PIXEL_BUFFER_H_
#define WATCHPANEL_PIXEL_BUFFER_H_

#include "raster.h"

#include <vector>

namespace watchpanel {

    // An in-memory Raster whose pixels can be read back out (GetPixel),
    // unlike a real display backend that only ever gets written to. Used
    // to render a Graphic off-screen so its pixels can be composited
    // elsewhere (e.g. cross-faded against another buffer) instead of
    // drawn straight to the real target.
    class PixelBuffer : public Raster {
    public:

        PixelBuffer(int width, int height);

        int Width() const override { return width_; }
        int Height() const override { return height_; }
        void Clear() override;
        void SetPixel(int x, int y, Color color) override;

        // Out-of-bounds reads return black rather than erroring, matching
        // SetPixel's silent-drop-on-out-of-bounds policy.
        Color GetPixel(int x, int y) const;

    private:

        int width_;
        int height_;
        std::vector<Color> pixels_;

    };

}

#endif // WATCHPANEL_PIXEL_BUFFER_H_
