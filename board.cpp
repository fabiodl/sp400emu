#include "board.h"
#include <chrono>
#include <iostream>

Board::Board(const char *rom)
    : buttons(0xFF), m68(rom), running(true), t([this] { run(); }) {}

Board::~Board() {
  running = false;
  t.join();
}

void Board::pushData(const uint8_t *d, size_t n) {
  std::lock_guard<std::mutex> lock(mutex);
  data.insert(data.end(), d, d + n);
}

void Board::pushData(const std::vector<uint8_t> &d) {
  std::lock_guard<std::mutex> lock(mutex);
  data.insert(data.end(), d.begin(), d.end());
}

void Board::setButtons(uint8_t b) {
  std::lock_guard<std::mutex> lock(mutex);
  buttons = b;
}

std::vector<PlotterState> Board::getStates() {
  std::lock_guard<std::mutex> lock(mutex);
  std::vector<PlotterState> s = states;
  states.clear();
  return s;
}

bool Board::isBusy() {
  std::lock_guard<std::mutex> lock(mutex);
  return data.empty() && !m68.getBusy();
}

void Board::run() {
  auto start = std::chrono::high_resolution_clock::now();
  bool pullData;
  uint8_t head;
  uint64_t t = 0;
  uint64_t ticks = 0;

  static const uint8_t MS = 8;
  while (running) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (!running) {
        return;
      }
      m68.getStates(states);
      pullData = !m68.getBusy() && !data.empty();
      if (pullData) {
        head = data.front();
        data.pop_front();
      }
      m68.setButtons(buttons & 0x07);
      if ((buttons & (1 << 4)) == 0) {
        m68.resetSystem();
      } else if ((buttons & (1 << 3)) == 0) {
        m68.resetCpu();
      }
    }

    if (pullData) {
      m68.pushData(head);
    }
    ticks += MS * 4000;
    t += MS / 2;
    m68.runToTime(ticks);
    std::this_thread::sleep_until(start + std::chrono::milliseconds(t));
    // auto
    // elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start).count();
    // std::cout<<"t"<<t<<" actual "<< elapsed<<std::endl;
  }
}
