#include "watchpanel/watchpanel.h"
#include "watchpanel/terminal_raster.h"

#include <ctime>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  const char *page = argc > 1 ? argv[1] : "configs/pages/weather.xml";
  const int width = argc > 2 ? std::stoi(argv[2]) : 64;
  const int height = argc > 3 ? std::stoi(argv[3]) : 64;
  const std::string fontPath = argc > 4 ? argv[4] : "fonts/tom-thumb.bdf";
  const std::string config = "configs/runtime/config.json";
  const std::string secrets = "configs/runtime/secrets.json";

  std::cout << "usage: ./build/simulator [page.xml] [width] [height] [font-bdf-path]" << std::endl;

  watchpanel::TerminalRaster raster(width, height);
  watchpanel::GraphicsContext context(&raster, fontPath);
  watchpanel::WatchPage wp(&context, config, secrets);
  if (wp.Load(page) != 0) {
    std::cerr << "failed to load page: " << page << std::endl;
    return 1;
  }

  const long now = static_cast<long>(std::time(nullptr));
  wp.Update(now, 0);
  wp.Draw();
  std::cout << raster.Render() << std::endl;
  return 0;
}
