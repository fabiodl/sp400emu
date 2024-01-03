#ifndef _BOARD_H_
#define _BOARD_H_

#include "m68sys.h"
#include <deque>
#include <mutex>
#include <thread>

class Board {
public:
  Board(const char *rom);
  ~Board();
  void pushData(const uint8_t *d, size_t n);
  void pushData(const std::vector<uint8_t> &d);
  void setButtons(uint8_t b);
  std::vector<PlotterState> getStates();
  bool isBusy();

private:
  uint8_t buttons;
  M68sys m68;
  void run();
  bool running;
  std::mutex mutex;
  std::thread t;
  std::vector<PlotterState> states;
  std::deque<uint8_t> data;
};

#endif
