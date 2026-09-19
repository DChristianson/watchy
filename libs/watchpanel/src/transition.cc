#include "transition.h"

#include <algorithm>
#include <cstdint>

namespace wpp = watchpanel;

wpp::FadeTransitionGraphic::FadeTransitionGraphic(
    GraphicsContext *context,
    int x,
    int y,
    int width,
    int height,
    long holdSeconds,
    long fadeSeconds,
    const std::string &fontPath,
    const std::string &cacheDir
) : Graphic(context),
    x(x),
    y(y),
    width(width),
    height(height),
    fromBuffer(width, height),
    toBuffer(width, height),
    fromContext(&fromBuffer, fontPath, cacheDir),
    toContext(&toBuffer, fontPath, cacheDir),
    fromGraphic(nullptr),
    toGraphic(nullptr),
    holdSeconds(holdSeconds),
    fadeSeconds(fadeSeconds),
    elapsedSeconds(0)
{}

wpp::FadeTransitionGraphic::~FadeTransitionGraphic() {
    delete fromGraphic;
    delete toGraphic;
    for (auto u : childUpdates) {
        delete u;
    }
}

void wpp::FadeTransitionGraphic::SetFromGraphic(Graphic *graphic) {
    delete fromGraphic;
    fromGraphic = graphic;
}

void wpp::FadeTransitionGraphic::SetToGraphic(Graphic *graphic) {
    delete toGraphic;
    toGraphic = graphic;
}

void wpp::FadeTransitionGraphic::AddChildUpdate(Updateable *update) {
    childUpdates.push_back(update);
}

double wpp::FadeTransitionGraphic::Progress() const {
    if (elapsedSeconds <= holdSeconds) return 0.0;
    if (fadeSeconds <= 0) return 1.0;
    const long fadeElapsed = elapsedSeconds - holdSeconds;
    if (fadeElapsed >= fadeSeconds) return 1.0;
    return static_cast<double>(fadeElapsed) / static_cast<double>(fadeSeconds);
}

void wpp::FadeTransitionGraphic::Update(const Model &model, long now, long deltaSeconds) {
    elapsedSeconds += deltaSeconds;
    for (auto u : childUpdates) {
        u->Update(model, now, deltaSeconds);
    }
}

namespace {

uint8_t Blend(uint8_t from, uint8_t to, double t) {
    return static_cast<uint8_t>(from + (static_cast<double>(to) - from) * t);
}

}  // namespace

void wpp::FadeTransitionGraphic::Draw() {
    fromBuffer.Clear();
    toBuffer.Clear();
    if (fromGraphic) fromGraphic->Draw();
    if (toGraphic) toGraphic->Draw();

    const double t = Progress();
    for (int py = 0; py < height; ++py) {
        for (int px = 0; px < width; ++px) {
            const Color a = fromBuffer.GetPixel(px, py);
            const Color b = toBuffer.GetPixel(px, py);
            const Color blended(Blend(a.r, b.r, t), Blend(a.g, b.g, t), Blend(a.b, b.b, t));
            context->SetPixel(x + px, y + py, blended);
        }
    }
}
