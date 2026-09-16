#ifndef WATCHPANEL_LED_CANVAS_H_
#define WATCHPANEL_LED_CANVAS_H_

#include "watchpanel/graphics.h"

#include <string>
#include <vector>

namespace watchpanel {

class LedCanvas : public Canvas {
 public:
  LedCanvas(int width, int height, const std::string &fontPath = "fonts/tom-thumb.bdf");
  ~LedCanvas() override;

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
  void Flush();

  int Width() const { return width_; }
  int Height() const { return height_; }
  const std::vector<uint8_t> &Pixels() const { return pixels_; }

 private:
  int width_;
  int height_;
  std::string fontPath_;
  std::vector<uint8_t> pixels_;

  void SetPixel(int x, int y, const Color &color);
  void DrawGlyph(unsigned char ch, const Color &color, int x, int y);
};

}  // namespace watchpanel

#endif  // WATCHPANEL_LED_CANVAS_H_
