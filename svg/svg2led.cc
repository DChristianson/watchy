
#include "watchpanel.h"
#include <iostream>

namespace wpp = watchpanel;

int main(int argc, char *argv[]) {
  wpp::WatchPage wp;
  wp.LoadPage(argv[1]);
  wp.Draw(NULL);
  return 0;
}

