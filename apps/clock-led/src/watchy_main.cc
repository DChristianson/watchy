#include "watchpanel/watchpanel.h"
#include "watchpanel/led_canvas.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef WATCHY_RGB_MATRIX
#include "led-matrix.h"
#endif

namespace {

std::atomic<bool> running(true);

void HandleShutdownSignal(int) { running = false; }

}  // namespace

int main(int argc, char **argv) {
  std::signal(SIGINT, HandleShutdownSignal);
  std::signal(SIGTERM, HandleShutdownSignal);

  int width = 64;
  int height = 64;

#ifdef WATCHY_RGB_MATRIX
  rgb_matrix::RGBMatrix::Options options;
  options.rows = height;
  options.cols = width;
  options.chain_length = 1;
  rgb_matrix::RuntimeOptions runtime;

  // Consumes any --led-* hardware flags (gpio mapping, rows/cols, daemon
  // mode, etc.) from argv, leaving our own args (just the page path) behind.
  rgb_matrix::RGBMatrix *matrix =
      rgb_matrix::RGBMatrix::CreateFromFlags(&argc, &argv, &options, &runtime);
  if (matrix == nullptr) {
    std::cerr << "clock-led: failed to initialize rpi-rgb-led-matrix hardware" << std::endl;
    return 1;
  }
  width = matrix->width();
  height = matrix->height();
  std::cout << "clock-led: driving real hardware (" << width << "x" << height << ")" << std::endl;
#else
  std::cout << "clock-led: WATCHY_ENABLE_RGB_MATRIX is off, rendering only (no hardware output). "
             << "Rebuild on the Pi with -DWATCHY_ENABLE_RGB_MATRIX=ON to drive real hardware."
             << std::endl;
#endif

  const char *page = argc > 1 ? argv[1] : "configs/pages/wordclock.xml";
  const std::string config = "configs/runtime/config.json";
  const std::string secrets = "configs/runtime/secrets.json";
  const long refreshMs = 1000;

  watchpanel::LedCanvas canvas(width, height);
  watchpanel::WatchPanel panel(&canvas, config, secrets);
  if (panel.Load(page) != 0) {
    std::cerr << "clock-led: failed to load panel page: " << page << std::endl;
#ifdef WATCHY_RGB_MATRIX
    delete matrix;
#endif
    return 1;
  }

#ifdef WATCHY_RGB_MATRIX
  canvas.SetFlushSink([matrix](int w, int h, const std::vector<uint8_t> &pixels) {
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        const int idx = (y * w + x) * 3;
        matrix->SetPixel(x, y, pixels[idx], pixels[idx + 1], pixels[idx + 2]);
      }
    }
  });
#endif

  while (running) {
    panel.Update();
    canvas.Clear();
    panel.Draw();
    canvas.Flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(refreshMs));
  }

#ifdef WATCHY_RGB_MATRIX
  matrix->Clear();
  delete matrix;
#endif
  std::cout << "clock-led: stopped" << std::endl;
  return 0;
}
