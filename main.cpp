#include "plotter.h"
#include "serial.h"
#include <iostream>

int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Usage " << argv[0] << " serial_dev" << std::endl;
    return 0;
  }

  Board board("sp400_6805.bin");

  Plotter plotter(board, 640, 480);
  Serial serial(board, argv[1]);

  plotter.run();

  return 0;
}
