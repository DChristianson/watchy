#ifndef WATCHY_TESTS_FAKE_RASTER_H_
#define WATCHY_TESTS_FAKE_RASTER_H_

#include "watchpanel/raster.h"

#include <algorithm>
#include <string>
#include <vector>

namespace watchy_test {

// Records every SetPixel call (both whether a pixel was touched and its
// actual color) into a plain grid, so a rendered frame can be compared as
// human-readable ASCII art (space = off, '*' = on) and/or checked for
// exact color values.
class FakeRaster : public watchpanel::Raster {
public:
    FakeRaster(int width, int height)
        : width_(width),
          height_(height),
          lit_(width * height, false),
          colorAt_(width * height, watchpanel::Color(0, 0, 0)) {}

    int Width() const override { return width_; }
    int Height() const override { return height_; }

    void Clear() override {
        std::fill(lit_.begin(), lit_.end(), false);
    }

    void SetPixel(int x, int y, watchpanel::Color color) override {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
        const int idx = y * width_ + x;
        lit_[idx] = true;
        colorAt_[idx] = color;
    }

    bool IsLit(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return false;
        return lit_[y * width_ + x];
    }

    watchpanel::Color ColorAt(int x, int y) const {
        if (x < 0 || y < 0 || x >= width_ || y >= height_) return watchpanel::Color(0, 0, 0);
        return colorAt_[y * width_ + x];
    }

    // Joins each row into one '\n'-separated string, so it can be compared
    // directly against an inline ASCII picture.
    std::string Render() const {
        return RenderRegion(0, 0, width_, height_);
    }

    // Same, but only over a sub-rectangle -- lets a test verify just the
    // part of a larger render it has hand-derived an expected picture for.
    std::string RenderRegion(int rx, int ry, int rw, int rh) const {
        std::string out;
        for (int y = ry; y < ry + rh; ++y) {
            for (int x = rx; x < rx + rw; ++x) {
                out += IsLit(x, y) ? '*' : ' ';
            }
            if (y + 1 < ry + rh) out += '\n';
        }
        return out;
    }

private:
    int width_;
    int height_;
    std::vector<bool> lit_;
    std::vector<watchpanel::Color> colorAt_;
};

}  // namespace watchy_test

#endif  // WATCHY_TESTS_FAKE_RASTER_H_
