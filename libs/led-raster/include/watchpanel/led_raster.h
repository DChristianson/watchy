#ifndef WATCHPANEL_LED_RASTER_H_
#define WATCHPANEL_LED_RASTER_H_

#include "watchpanel/raster.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace watchpanel {

class LedRaster : public Raster {
 public:
  // Called by Flush() with the rasterized RGB24 pixel buffer (row-major,
  // 3 bytes per pixel). Lets a hardware-specific driver (e.g. clock-led's
  // rpi-rgb-led-matrix integration) consume the frame without LedRaster
  // itself depending on any hardware library.
  using FlushSink = std::function<void(int width, int height, const std::vector<uint8_t> &pixels)>;

  LedRaster(int width, int height);
  ~LedRaster() override;

  int Width() const override { return width_; }
  int Height() const override { return height_; }
  void Clear() override;
  void SetPixel(int x, int y, Color color) override;

  void SetFlushSink(FlushSink sink);
  void Flush();

  const std::vector<uint8_t> &Pixels() const { return pixels_; }

 private:
  int width_;
  int height_;
  std::vector<uint8_t> pixels_;
  FlushSink flushSink_;
};

}  // namespace watchpanel

#endif  // WATCHPANEL_LED_RASTER_H_
