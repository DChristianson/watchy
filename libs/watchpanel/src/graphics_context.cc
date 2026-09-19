#include "graphics_context.h"
#include "watchpanel/bdf_font.h"
#include "watchpanel/image_decode.h"
#include "hamper.h"

#include <algorithm>
#include <ctime>
#include <sstream>

namespace wpp = watchpanel;

wpp::GraphicsContext::GraphicsContext(Raster *raster, const std::string &fontPath,
                                       const std::string &cacheDir)
    : raster(raster), fontPath(fontPath), cacheDir(cacheDir) {}

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

int wpp::GraphicsContext::DrawText(
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
    Overflow overflow,
    int scrollOffsetY)
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
    const int contentHeight = static_cast<int>(lines.size()) * effectiveLineOffset;

    // A non-zero scroll offset needs the box to act as a clipping viewport
    // even if the page didn't ask for overflow="clip" -- otherwise the
    // lines scrolled above the box would still be drawn past its top edge.
    const bool clipActive = (overflow == Overflow::kClip || scrollOffsetY != 0) && width > 0 && height > 0;
    const ClipBox clip{clipActive, x, y, width, height};

    int cursorY = y - scrollOffsetY;
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

    return contentHeight;
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
    const char *href,
    long maxAgeSeconds)
{
    const std::string hrefStr(href ? href : "");
    const bool isRemote = hrefStr.rfind("http://", 0) == 0 || hrefStr.rfind("https://", 0) == 0;

    // Remote URLs go through hamper's cache first -- fetched at most once
    // per maxAgeSeconds, and a failed refresh still falls back to
    // whatever was last fetched successfully rather than failing here.
    // Local paths are used as-is.
    //
    // Draw() (unlike Update()) doesn't carry an injected timestamp through
    // its call chain, so this is the one remaining real std::time() read
    // in the pipeline -- everything time-related that happens during
    // Update() (feed fetches, TimeData, flip timing) is fully injectable.
    std::string localPath = hrefStr;
    if (isRemote) {
        const long now = static_cast<long>(std::time(nullptr));
        localPath = hamper::fetch_image(hrefStr.c_str(), now, maxAgeSeconds, cacheDir.c_str());
    }

    // Decode the real image (PNG/JPEG via stb_image) and nearest-neighbor
    // scale it to fit the declared box, preserving aspect ratio and
    // centered (letterboxed) -- matches the pixelated, chunky look already
    // used elsewhere (SvgRaster's scaled-up glyphs) rather than a
    // smoothing resize filter.
    DecodedImage decoded;
    if (!localPath.empty() && DecodeImage(localPath, &decoded) &&
        decoded.width > 0 && decoded.height > 0) {
        const double scale = std::min(
            static_cast<double>(width) / decoded.width,
            static_cast<double>(height) / decoded.height);
        const int drawWidth = std::max(1, static_cast<int>(decoded.width * scale));
        const int drawHeight = std::max(1, static_cast<int>(decoded.height * scale));
        const int offsetX = x + (width - drawWidth) / 2;
        const int offsetY = y + (height - drawHeight) / 2;

        for (int dy = 0; dy < drawHeight; ++dy) {
            const int srcY = std::min(decoded.height - 1,
                                       static_cast<int>(static_cast<double>(dy) * decoded.height / drawHeight));
            for (int dx = 0; dx < drawWidth; ++dx) {
                const int srcX = std::min(decoded.width - 1,
                                           static_cast<int>(static_cast<double>(dx) * decoded.width / drawWidth));
                const uint8_t *pixel = decoded.At(srcX, srcY);
                const uint8_t alpha = pixel[3];
                if (alpha < 128) continue;  // mostly-transparent source pixel: leave untouched
                raster->SetPixel(offsetX + dx, offsetY + dy, Color(pixel[0], pixel[1], pixel[2]));
            }
        }
        return;
    }

    // Nothing usable: the fetch failed with no cache to fall back to, or
    // the file couldn't be decoded. Draw a plain placeholder block.
    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            raster->SetPixel(px, py, Color(255, 255, 255));
        }
    }
}

void wpp::GraphicsContext::SetPixel(int x, int y, Color color) {
    raster->SetPixel(x, y, color);
}
