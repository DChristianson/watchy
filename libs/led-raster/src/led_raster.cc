#include "watchpanel/led_raster.h"

#include <algorithm>
#include <utility>

namespace watchpanel {

LedRaster::LedRaster(int width, int height)
    : width_(width), height_(height), pixels_(width * height * 3, 0) {}

LedRaster::~LedRaster() {}

void LedRaster::Clear() {
  std::fill(pixels_.begin(), pixels_.end(), 0);
}

void LedRaster::SetPixel(int x, int y, Color color) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
  const int idx = (y * width_ + x) * 3;
  pixels_[idx + 0] = color.r;
  pixels_[idx + 1] = color.g;
  pixels_[idx + 2] = color.b;
}

void LedRaster::SetFlushSink(FlushSink sink) {
  flushSink_ = std::move(sink);
}

void LedRaster::Flush() {
  if (flushSink_) {
    flushSink_(width_, height_, pixels_);
  }
}

}  // namespace watchpanel
