#ifndef SP400_SERIAL_H_
#define SP400_SERIAL_H_

#include "board.h"
#include <thread>

class Serial {
public:
  Serial(Board &board, const std::string &dev);
  ~Serial();

private:
  void run();
  void setRts(bool b);
  Board &board;
  std::string dev;
  bool running;
  std::thread t;
  int fd;
};

#endif
