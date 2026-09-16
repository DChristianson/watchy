#ifndef WATCHPANEL_TERMINAL_CANVAS_H_
#define WATCHPANEL_TERMINAL_CANVAS_H_

#include "watchpanel/graphics.h"

#include <string>
#include <vector>

namespace watchpanel {

// Renders a page to a colored ASCII grid for terminal preview, using the
// same BDF glyph rasterization as LedCanvas so the two stay visually
// consistent.
class TerminalCanvas : public Canvas {
 public:
  TerminalCanvas(int width, int height, const std::string &fontPath = "fonts/tom-thumb.bdf");
  ~TerminalCanvas() override;

  void DrawText(
      const TextSpan *textSpan,
      const char *fontName,
      Color color,
      int x,
      int y,
      int letterSpacing,
      int lineOffset) override;

  void DrawRect(
      int x,
      int y,
      int width,
      int height,
      Color fill,
      Color stroke) override;

  void DrawImage(
      int x,
      int y,
      int width,
      int height,
      const char *href) override;

  void Clear();

  // Renders the current frame as text; ANSI truecolor escapes when useColor
  // is true (the default), plain "[]"/".." otherwise.
  std::string Render(bool useColor = true) const;

  int Width() const { return width_; }
  int Height() const { return height_; }

 private:
  int width_;
  int height_;
  std::string fontPath_;
  std::vector<bool> lit_;
  std::vector<Color> colorAt_;

  int Index(int x, int y) const { return y * width_ + x; }
  void SetPixel(int x, int y, const Color &color);
  void DrawGlyph(unsigned char ch, const Color &color, int x, int y);
};

}  // namespace watchpanel

#endif  // WATCHPANEL_TERMINAL_CANVAS_H_
