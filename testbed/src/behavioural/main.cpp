#include "../behavioural/top.h"

#include "common/P4.h"

int pfp_main(int pfp_argc, char* pfp_argv[]) {
  setP4LoggingLevels();

  auto Top = std::make_shared<top>("top");
  Top->init();

  sc_start();
  return 0;
}
