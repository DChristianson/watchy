#ifndef WATCHPANEL_IMAGE_DECODE_H_
#define WATCHPANEL_IMAGE_DECODE_H_

#include <cstdint>
#include <string>
#include <vector>

namespace watchpanel {

struct DecodedImage {
    int width = 0;
    int height = 0;
    // RGBA8, row-major, 4 bytes per pixel (stb_image fills alpha=255 for
    // source formats that have no alpha channel of their own).
    std::vector<uint8_t> pixels;

    // Convenience accessor for a single pixel's RGBA bytes.
    const uint8_t *At(int x, int y) const {
        return pixels.data() + (static_cast<size_t>(y) * width + x) * 4;
    }
};

// Decodes a PNG or JPEG file (format auto-detected from its signature) into
// RGBA8 pixels via stb_image. Returns false if the file doesn't exist or
// isn't a format stb_image recognizes.
bool DecodeImage(const std::string &path, DecodedImage *out);

}  // namespace watchpanel

#endif  // WATCHPANEL_IMAGE_DECODE_H_
