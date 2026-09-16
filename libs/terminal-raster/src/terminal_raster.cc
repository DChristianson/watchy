#include "watchpanel/terminal_raster.h"

#include <algorithm>
#include <sstream>

namespace watchpanel {

TerminalRaster::TerminalRaster(int width, int height)
    : width_(width),
      height_(height),
      lit_(width * height, false),
      colorAt_(width * height, Color(0, 0, 0)) {}

TerminalRaster::~TerminalRaster() {}

void TerminalRaster::Clear() {
  std::fill(lit_.begin(), lit_.end(), false);
}

void TerminalRaster::SetPixel(int x, int y, Color color) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
  const int idx = Index(x, y);
  lit_[idx] = true;
  colorAt_[idx] = color;
}

std::string TerminalRaster::Render(bool useColor) const {
  std::ostringstream out;
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      const int idx = Index(x, y);
      if (lit_[idx]) {
        const Color &c = colorAt_[idx];
        if (useColor) {
          out << "\033[38;2;" << static_cast<int>(c.r) << ";" << static_cast<int>(c.g) << ";"
              << static_cast<int>(c.b) << "m[]\033[0m";
        } else {
          out << "[]";
        }
      } else {
        if (useColor) {
          out << "\033[38;5;236m..\033[0m";
        } else {
          out << "..";
        }
      }
    }
    out << "\n";
  }
  return out.str();
}

}  // namespace watchpanel
