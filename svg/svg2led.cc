
#include "watchpanel.h"
#include "svg.h"

#include <iostream>

namespace wpp = watchpanel;

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    const char *file = argv[i];
    wpp::SvgCanvas canvas(64, 64);
    wpp::WatchPage wp(&canvas);
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

