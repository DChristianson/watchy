#ifndef WATCHPANEL_TERMINAL_RASTER_H_
#define WATCHPANEL_TERMINAL_RASTER_H_

#include "watchpanel/raster.h"

#include <string>
#include <vector>

namespace watchpanel {

// Renders a page to a colored ASCII grid for terminal preview.
class TerminalRaster : public Raster {
 public:
  TerminalRaster(int width, int height);
  ~TerminalRaster() override;

  int Width() const override { return width_; }
  int Height() const override { return height_; }
  void Clear() override;
  void SetPixel(int x, int y, Color color) override;

  // Renders the current frame as text; ANSI truecolor escapes when useColor
  // is true (the default), plain "[]"/".." otherwise.
  std::string Render(bool useColor = true) const;

 private:
  int width_;
  int height_;
  std::vector<bool> lit_;
  std::vector<Color> colorAt_;

  int Index(int x, int y) const { return y * width_ + x; }
};

}  // namespace watchpanel

#endif  // WATCHPANEL_TERMINAL_RASTER_H_
