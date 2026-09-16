#include "watchpanel/watchpanel.h"
#include "watchpanel/svg_canvas.h"

#include <iostream>

int main(int argc, char **argv) {
  const char *page = argc > 1 ? argv[1] : "configs/pages/weather.xml";
  std::string config = "configs/runtime/config.json";
  std::string secrets = "configs/runtime/secrets.json";

  watchpanel::SvgCanvas canvas(64, 64);
  watchpanel::WatchPanel panel(&canvas, config, secrets);
  if (panel.Load(page) != 0) {
    std::cerr << "failed to load panel page: " << page << std::endl;
    return 1;
  }
  panel.Update();
  panel.Draw();
  canvas.Save("./out.svg");
  std::cout << "wrote out.svg" << std::endl;
  return 0;
}
