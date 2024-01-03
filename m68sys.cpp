#include "m68sys.h"
#include <fstream>
#include <iostream>
#include <sstream>
enum { PORTA = 0, PORTB = 1, PORTC = 2, PORTD = 3 };

extern "C" {
static uint8_t readfunc(struct M68_CTX *ctx, const uint16_t addr);
static void writefunc(struct M68_CTX *ctx, const uint16_t addr,
                      const uint8_t data);
}

PlotterState::PlotterState(int32_t _x, int32_t _y, bool _penDown,
                           uint8_t _colorIdx, bool _busy)
    : x(_x), y(_y), penDown(_penDown), colorIdx(_colorIdx), busy(_busy) {}

std::ostream &operator<<(std::ostream &o, const PlotterState &p) {
  o << p.x << " " << p.y << " " << (p.penDown ? 'd' : 'u') << " "
    << (int)p.colorIdx;
  return o;
}

bool PlotterState::operator!=(const PlotterState &b) {
  return b.x != x || b.y != y || b.penDown != penDown ||
         b.colorIdx != colorIdx || b.busy != busy;
}

M68sys::M68sys(const char *rom)
    : colorPhase(0), time(0), buttons(0b111), porta(0x80), portc(0x84) {
  std::ifstream binaryFile(rom, std::ios::in | std::ios::binary);
  if (binaryFile.is_open()) {
    binaryFile.read(reinterpret_cast<char *>(memspace.data()), arraySize);
  } else {
    std::stringstream ss;
    ss << "Error in opening file " << rom << std::endl;
    throw std::runtime_error(ss.str());
  }

  read_mem = &readfunc;
  write_mem = &writefunc;
  opdecode = NULL;
  m68_init(this, M68_CPU_HD6805V1);
  trace = false;
  m68_reset(this);
  tmr_init(&tmr);
  stepper_init(&xstep);
  stepper_init(&ystep);
}

uint8_t M68sys::read(uint16_t addr) {
  if (addr == PORTA) {
    return porta | (buttons << 2);
  }

  uint8_t v = memspace[addr];
  if (addr == PORTC) {
    return (v & ~((1 << 2) | (1 << 7))) | portc | (getReed() << 3);
  }

  tmr_read(&tmr, addr, &v);
  return v;
}

bool M68sys::getBusy() { return busy; }

void M68sys::pushState() {
  static PlotterState prev(0, 0, false, 0, false);

  PlotterState ps(xstep.pos, ystep.pos, penDown, colorPhase / 3, busy);
  if (ps != prev) {
    prev = ps;
    commands.emplace_back(ps);
    // std::cout<<"state "<<std::dec<<ps<<std::endl;
  }
}

void M68sys::write(uint16_t addr, uint8_t data) {
  memspace[addr] = data;
  if (addr == PORTB) {
    int32_t xpos = xstep.pos;
    stepper_update_rev(&xstep, data & 0x0F);
    stepper_update(&ystep, data >> 4);

    if ((xpos >= METALTABPOS) && (xstep.pos < METALTABPOS)) {
      colorPhase = (colorPhase + 1) % 12;
      std::cout << "colorPhase" << (int)colorPhase << std::endl;
    }

    pushState();

  } else if (addr == PORTC) {
    if ((data & 1) == 0) {
      penDown = false;
    }
    if ((data & 2) == 0) {
      penDown = true;
    }

    busy = data & (1 << 4);
    pushState();
  }
  tmr_write(&tmr, addr, data);
}

extern "C" {
uint8_t readfunc(struct M68_CTX *ctx, const uint16_t addr) {
  return static_cast<M68sys *>(ctx)->read(addr);
}

void writefunc(struct M68_CTX *ctx, const uint16_t addr, const uint8_t data) {
  static_cast<M68sys *>(ctx)->write(addr, data);
}
}

void M68sys::setButtons(uint8_t v) { buttons = v; }

bool M68sys::getReed() { return colorPhase == 0 && xstep.pos < METALTABPOS; }

void M68sys::pushData(uint8_t d) {
  memspace[PORTD] = d;
  m68_set_interrupt_line(this, M68_INT_IRQ);
}

uint64_t M68sys::step() {
  uint64_t cycles = m68_exec_cycle(this);
  time += cycles;
  // std::cout<<"time"<<time<<std::endl;
  if (tmr_exec(&tmr, cycles, false)) {
    m68_set_interrupt_line(this, M68_INT_TIMER1);
    // std::cout<<"timer"<<std::endl;
  }
  return cycles;
}

/*extern "C"{
  uint64_t globalt;
  }*/

void M68sys::runToTime(uint64_t t) {
  while (time < t) {
    // globalt=time;
    uint64_t cycles = m68_exec_cycle(this);
    time += cycles;
    // std::cout<<"time"<<time<<std::endl;
    if (tmr_exec(&tmr, cycles, false)) {
      m68_set_interrupt_line(this, M68_INT_TIMER1);
      // std::cout<<"timer"<<std::endl;
    }
  }
}

void M68sys::getStates(std::vector<PlotterState> &states) {
  states.insert(states.end(), commands.begin(), commands.end());
  /*if (commands.size()){
    std::cout<<commands.size()<<"commands"<<std::endl;
  }
  if (states.size()){
    std::cout<<states.size()<<"states"<<std::endl;
    }*/
  commands.clear();
}

void M68sys::resetCpu() {
  std::cout << "reset cpu" << std::endl;
  m68_init(this, M68_CPU_HD6805V1);
  tmr_init(&tmr);
}

void M68sys::resetSystem() {
  std::cout << "reset system" << std::endl;
  resetCpu();
  stepper_init(&xstep);
  stepper_init(&ystep);
  colorPhase = 0;
}

void M68sys::setConfig(uint8_t _porta, uint8_t _portc) {
  porta = _porta;
  portc = _portc;
}
