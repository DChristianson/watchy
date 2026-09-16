#include "watchpanel/watchpanel.h"
#include "watchpanel/terminal_canvas.h"

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

  watchpanel::TerminalCanvas canvas(width, height, fontPath);
  watchpanel::WatchPage wp(&canvas, config, secrets);
  if (wp.Load(page) != 0) {
    std::cerr << "failed to load page: " << page << std::endl;
    return 1;
  }

  wp.Update();
  wp.Draw();
  std::cout << canvas.Render() << std::endl;
  return 0;
}
