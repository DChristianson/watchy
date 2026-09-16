
#include "watchpanel/watchpanel.h"
#include "watchpanel/svg_raster.h"

#include <iostream>
#include <string>

namespace wpp = watchpanel;

int main(int argc, char *argv[]) {
  std::string config = "configs/runtime/config.json";
  std::string secrets = "configs/runtime/secrets.json";

  for (int i = 1; i < argc; i++) {
    const char *file = argv[i];
    wpp::SvgRaster raster(64, 64);
    wpp::GraphicsContext context(&raster);
    wpp::WatchPage wp(&context, config, secrets);
    std::cout << "Loading " << file << std::endl;
    wp.Load(file);
    std::cout << "Updating..." << std::endl;
    wp.Update();
    std::cout << "Drawing..." << std::endl;
    wp.Draw();
    std::cout << "Saving..." << std::endl;
    std::string out(file);
    out += ".svg";
    raster.Save(out.c_str());
  }
  std::cout << "done" << std::endl;
  return 0;
}
