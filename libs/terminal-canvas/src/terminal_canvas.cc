#include "watchpanel/terminal_canvas.h"
#include "watchpanel/bdf_font.h"

#include <algorithm>
#include <sstream>

namespace watchpanel {

TerminalCanvas::TerminalCanvas(int width, int height, const std::string &fontPath)
    : width_(width),
      height_(height),
      fontPath_(fontPath),
      lit_(width * height, false),
      colorAt_(width * height, Color(0, 0, 0)) {}

TerminalCanvas::~TerminalCanvas() {}

void TerminalCanvas::Clear() {
  std::fill(lit_.begin(), lit_.end(), false);
}

void TerminalCanvas::SetPixel(int x, int y, const Color &color) {
  if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
  const int idx = Index(x, y);
  lit_[idx] = true;
  colorAt_[idx] = color;
}

void TerminalCanvas::DrawGlyph(unsigned char ch, const Color &color, int x, int y) {
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

void TerminalCanvas::DrawText(
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

void TerminalCanvas::DrawRect(
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

void TerminalCanvas::DrawImage(
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

std::string TerminalCanvas::Render(bool useColor) const {
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
