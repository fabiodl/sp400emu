#ifndef _M68SYS_H_
#define _M68SYS_H_
#include <array>
#include <cstddef>
#include <functional>
#include <ostream>
#include <stdbool.h>
#include <stdint.h>
#include <vector>

extern "C" {
#include "stepper.h"
#include <m68emu/m68emu.h>
#include <m68emu/m68tmr.h>
}

struct PlotterState {
  int32_t x, y;
  bool penDown;
  uint8_t colorIdx;
  bool busy;
  PlotterState(int32_t x, int32_t y, bool penDown, uint8_t colorIdx, bool busy);
  bool operator!=(const PlotterState &b);
};

std::ostream &operator<<(std::ostream &, const PlotterState &);

class M68sys : public M68_CTX {
public:
  M68sys(const char *rom);

  void setButtons(uint8_t v);
  void pushData(uint8_t c);
  void runToTime(uint64_t t);
  void getStates(std::vector<PlotterState> &s);
  void resetCpu();
  void resetSystem();

  uint8_t read(uint16_t addr);
  void write(uint16_t addr, uint8_t val);
  bool getBusy();
  uint64_t step();
  void setConfig(uint8_t porta, uint8_t portc);

protected:
  std::vector<PlotterState> commands;

private:
  static constexpr size_t arraySize = 0x1000;
  static constexpr int32_t METALTABPOS = -523;
  std::array<uint8_t, arraySize> memspace;
  M68TMR_CTX tmr;
  STEPPER xstep, ystep;
  bool penDown;
  uint8_t colorPhase;
  uint64_t time;
  uint8_t buttons;
  bool busy;
  uint8_t porta, portc;
  bool getReed();
  void pushState();
};

#endif
