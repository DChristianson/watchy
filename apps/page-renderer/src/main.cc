
#include "watchpanel/watchpanel.h"
#include "watchpanel/svg_canvas.h"

#include <iostream>
#include <string>

namespace wpp = watchpanel;

int main(int argc, char *argv[]) {
  std::string config = "configs/runtime/config.json";
  std::string secrets = "configs/runtime/secrets.json";

  for (int i = 1; i < argc; i++) {
    const char *file = argv[i];
    wpp::SvgCanvas canvas(64, 64);
    wpp::WatchPage wp(&canvas, config, secrets);
    std::cout << "Loading " << file << std::endl;
    wp.Load(file);
    std::cout << "Updating..." << std::endl;
    wp.Update();
    std::cout << "Drawing..." << std::endl;
    wp.Draw();
    std::cout << "Saving..." << std::endl;
    std::string out(file);
    out += ".svg";
    canvas.Save(out.c_str());
  }
  std::cout << "done" << std::endl;
  return 0;
}

