#include "watchpanel/image_decode.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

namespace watchpanel {

bool DecodeImage(const std::string &path, DecodedImage *out) {
    int width = 0;
    int height = 0;
    int sourceChannels = 0;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &sourceChannels, 4);
    if (!data) return false;

    out->width = width;
    out->height = height;
    out->pixels.assign(data, data + (static_cast<size_t>(width) * height * 4));
    stbi_image_free(data);
    return true;
}

}  // namespace watchpanel
