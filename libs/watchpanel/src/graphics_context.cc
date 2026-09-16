#include "graphics_context.h"
#include "watchpanel/bdf_font.h"

#include <algorithm>
#include <sstream>

namespace wpp = watchpanel;

wpp::GraphicsContext::GraphicsContext(Raster *raster, const std::string &fontPath)
    : raster(raster), fontPath(fontPath) {}

int wpp::GraphicsContext::GlyphAdvance(unsigned char ch) const {
    if (fontPath.empty()) return 4;
    const BdfFont &font = BdfFont::Load(fontPath);
    const BdfGlyph *glyph = font.Find(static_cast<int>(ch));
    if (!glyph) return 4;
    // DWIDTH is the real advance; ink width is only a fallback for glyphs
    // that somehow lack it.
    return glyph->dwidth > 0 ? glyph->dwidth : glyph->width;
}

void wpp::GraphicsContext::DrawGlyph(unsigned char ch, Color color, int x, int baselineY, const ClipBox &clip) {
    if (fontPath.empty()) return;
    const BdfFont &font = BdfFont::Load(fontPath);
    const BdfGlyph *glyph = font.Find(static_cast<int>(ch));
    if (!glyph) return;

    // BBX's xOffset/yOffset position the glyph's bitmap relative to the
    // baseline origin, not its top-left — this is what makes descenders
    // (g, p, y, ...) actually drop below the baseline.
    const int originX = x + glyph->xOffset;
    const int topY = baselineY - glyph->yOffset - glyph->height;

    for (int gy = 0; gy < glyph->height; ++gy) {
        for (int gx = 0; gx < glyph->width; ++gx) {
            if (!glyph->GetBit(gx, gy)) continue;
            const int px = originX + gx;
            const int py = topY + gy;
            if (clip.active &&
                (px < clip.x || px >= clip.x + clip.width || py < clip.y || py >= clip.y + clip.height)) {
                continue;
            }
            raster->SetPixel(px, py, color);
        }
    }
}

std::vector<std::string> wpp::GraphicsContext::WrapWords(
    const std::string &text, int maxWidthPx, int letterSpacing) const {
    std::vector<std::string> lines;
    if (maxWidthPx <= 0) {
        lines.push_back(text);
        return lines;
    }

    auto textWidth = [&](const std::string &s) {
        int w = 0;
        for (size_t i = 0; i < s.size(); ++i) {
            w += GlyphAdvance(static_cast<unsigned char>(s[i]));
            if (i + 1 < s.size()) w += letterSpacing;
        }
        return w;
    };

    std::istringstream words(text);
    std::string word;
    std::string current;

    while (words >> word) {
        // A single word wider than the line on its own gets hard-broken.
        while (textWidth(word) > maxWidthPx && word.size() > 1) {
            size_t fit = 1;
            while (fit < word.size() && textWidth(word.substr(0, fit + 1)) <= maxWidthPx) {
                ++fit;
            }
            if (!current.empty()) {
                lines.push_back(current);
                current.clear();
            }
            lines.push_back(word.substr(0, fit));
            word = word.substr(fit);
        }

        const std::string candidate = current.empty() ? word : current + " " + word;
        if (!current.empty() && textWidth(candidate) > maxWidthPx) {
            lines.push_back(current);
            current = word;
        } else {
            current = candidate;
        }
    }
    if (!current.empty() || lines.empty()) {
        lines.push_back(current);
    }
    return lines;
}

void wpp::GraphicsContext::DrawText(
    const TextSpan *textSpan,
    const char *fontName,
    Color color,
    int x,
    int y,
    int width,
    int height,
    int letterSpacing,
    int lineOffset,
    Wrap wrap,
    Overflow overflow)
{
    (void)fontName;

    std::vector<std::string> lines;
    for (const TextSpan *span = textSpan; span; span = span->nextSpan) {
        if (wrap == Wrap::kWord && width > 0) {
            const std::vector<std::string> wrapped = WrapWords(span->text, width, letterSpacing);
            lines.insert(lines.end(), wrapped.begin(), wrapped.end());
        } else {
            lines.push_back(span->text);
        }
    }

    // Lines need real vertical spacing to be distinguishable; if no
    // explicit line-offset is given, fall back to the tallest glyph
    // actually used plus a 1px gap rather than collapsing every line onto
    // the same y. Same fallback for the font's ascent, in case it's
    // missing FONT_ASCENT.
    int maxGlyphHeight = 1;
    int fontAscent = 0;
    if (fontPath.size()) {
        const BdfFont &font = BdfFont::Load(fontPath);
        fontAscent = font.Ascent();
        for (const auto &line : lines) {
            for (unsigned char ch : line) {
                if (const BdfGlyph *glyph = font.Find(ch)) {
                    maxGlyphHeight = std::max(maxGlyphHeight, glyph->height);
                }
            }
        }
    }
    if (fontAscent <= 0) {
        fontAscent = maxGlyphHeight;
    }
    const int effectiveLineOffset = lineOffset > 0 ? lineOffset : maxGlyphHeight + 1;

    const ClipBox clip{overflow == Overflow::kClip && width > 0 && height > 0, x, y, width, height};

    int cursorY = y;
    for (const auto &line : lines) {
        if (clip.active && cursorY >= y + height) break;
        const int baselineY = cursorY + fontAscent;
        int cursorX = x;
        for (size_t i = 0; i < line.size(); ++i) {
            const unsigned char ch = static_cast<unsigned char>(line[i]);
            DrawGlyph(ch, color, cursorX, baselineY, clip);
            cursorX += GlyphAdvance(ch) + letterSpacing;
        }
        cursorY += effectiveLineOffset;
    }
}

void wpp::GraphicsContext::DrawRect(
    int x,
    int y,
    int width,
    int height,
    Color fill,
    Color stroke)
{
    (void)stroke;  // Outline-only stroke isn't implemented; matches prior solid-fill behavior.
    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            raster->SetPixel(px, py, fill);
        }
    }
}

void wpp::GraphicsContext::DrawImage(
    int x,
    int y,
    int width,
    int height,
    const char *href)
{
    (void)href;
    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            raster->SetPixel(px, py, Color(255, 255, 255));
        }
    }
}
