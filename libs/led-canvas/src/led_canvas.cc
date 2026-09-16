#include "watchpanel/led_canvas.h"
#include "watchpanel/bdf_font.h"

#include <algorithm>

namespace watchpanel {

LedCanvas::LedCanvas(int width, int height, const std::string &fontPath)
    : width_(width), height_(height), fontPath_(fontPath), pixels_(width * height * 3, 0) {}

LedCanvas::~LedCanvas() {}

void LedCanvas::Clear() {
  std::fill(pixels_.begin(), pixels_.end(), 0);
}

void LedCanvas::SetPixel(int x, int y, const Color &color) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
  const int idx = (y * width_ + x) * 3;
  pixels_[idx + 0] = color.r;
  pixels_[idx + 1] = color.g;
  pixels_[idx + 2] = color.b;
}

void LedCanvas::DrawGlyph(unsigned char ch, const Color &color, int x, int y) {
  if (fontPath_.empty()) return;
  const BdfFont &font = BdfFont::Load(fontPath_);
  const BdfGlyph *glyph = font.Find(static_cast<int>(ch));
  if (!glyph) return;

  for (int gy = 0; gy < glyph->height; ++gy) {
    for (int gx = 0; gx < glyph->width; ++gx) {
      if (glyph->GetBit(gx, gy)) SetPixel(x + gx, y + gy, color);
    }
  }
}

void LedCanvas::DrawText(
    const TextSpan *textSpan,
    const char *fontName,
    Color color,
    int x,
    int y,
    int letterSpacing,
    int lineOffset) {
  (void)fontName;
  (void)lineOffset;
  int cursorX = x;
  while (textSpan) {
    const std::string &s = textSpan->text;
    for (size_t i = 0; i < s.size(); ++i) {
      const unsigned char ch = static_cast<unsigned char>(s[i]);
      DrawGlyph(ch, color, cursorX, y);
      cursorX += 4 + letterSpacing;
    }
    textSpan = textSpan->nextSpan;
    if (textSpan) {
      cursorX += 2;
    }
  }
}

void LedCanvas::DrawRect(
    int x,
    int y,
    int width,
    int height,
    Color fill,
    Color stroke) {
  (void)stroke;
  for (int py = y; py < y + height; ++py) {
    for (int px = x; px < x + width; ++px) {
      SetPixel(px, py, fill);
    }
  }
}

void LedCanvas::DrawImage(
    int x,
    int y,
    int width,
    int height,
    const char *href) {
  (void)href;
  for (int py = y; py < y + height; ++py) {
    for (int px = x; px < x + width; ++px) {
      SetPixel(px, py, Color(255, 255, 255));
    }
  }
}

void LedCanvas::Flush() {
  // Hardware-specific emission is intentionally left as a stub here.
  // This class implements the Canvas contract and exposes the rasterized pixel buffer.
}

}  // namespace watchpanel
