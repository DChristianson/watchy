#ifndef WATCHPANEL_LED_CANVAS_H_
#define WATCHPANEL_LED_CANVAS_H_

#include "watchpanel/graphics.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace watchpanel {

class LedCanvas : public Canvas {
 public:
  // Called by Flush() with the rasterized RGB24 pixel buffer (row-major,
  // 3 bytes per pixel). Lets a hardware-specific driver (e.g. clock-led's
  // rpi-rgb-led-matrix integration) consume the frame without LedCanvas
  // itself depending on any hardware library.
  using FlushSink = std::function<void(int width, int height, const std::vector<uint8_t> &pixels)>;

  LedCanvas(int width, int height, const std::string &fontPath = "fonts/tom-thumb.bdf");
  ~LedCanvas() override;

  void SetFlushSink(FlushSink sink);

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
  FlushSink flushSink_;

  void SetPixel(int x, int y, const Color &color);
  void DrawGlyph(unsigned char ch, const Color &color, int x, int y);
};

}  // namespace watchpanel

#endif  // WATCHPANEL_LED_CANVAS_H_
