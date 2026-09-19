#include "pixel_buffer.h"

#include <algorithm>

namespace wpp = watchpanel;

wpp::PixelBuffer::PixelBuffer(int width, int height)
    : width_(width),
      height_(height),
      pixels_(static_cast<size_t>(width) * static_cast<size_t>(height), Color(0, 0, 0)) {}

void wpp::PixelBuffer::Clear() {
    std::fill(pixels_.begin(), pixels_.end(), Color(0, 0, 0));
}

void wpp::PixelBuffer::SetPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    pixels_[static_cast<size_t>(y) * width_ + x] = color;
}

wpp::Color wpp::PixelBuffer::GetPixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return Color(0, 0, 0);
    return pixels_[static_cast<size_t>(y) * width_ + x];
}
