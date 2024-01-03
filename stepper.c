#include "stepper.h"
//#include <stdio.h>

void stepper_init(STEPPER *s) {
  s->phase = 0;
  s->pos = 0;
}

// 6,5,9,a

uint8_t phase(uint8_t v) {
  switch (v) {
  case 6:
    return 0;
  case 5:
    return 1;
  case 9:
    return 2;
  case 0x0A:
    return 3;
  default:
    // printf("unknown state %x",v);
    return 0x80;
  }
}

void stepper_update(STEPPER *s, uint8_t pattern) {
  if (!pattern)
    return;
  uint8_t p1 = phase(pattern);
  uint8_t jump = (p1 - s->phase) & 0x03;
  switch (jump) {
  case 1:
    s->pos++;
    // printf("%p step +\n",s);
    break;
  case 3:
    s->pos--;
    // printf("%p step -\n",s);
    break;
  };
  s->phase = p1;
}

extern uint64_t globalt;

void stepper_update_rev(STEPPER *s, uint8_t pattern) {
  if (!pattern)
    return;
  uint8_t p1 = phase(pattern);
  uint8_t jump = (p1 - s->phase) & 0x03;
  // static uint64_t prevt;
  switch (jump) {
  case 1:
    s->pos--;
    //    printf("%ld %p step -\n",globalt-prevt,s);
    // prevt=globalt;
    break;
  case 3:
    s->pos++;
    // printf("%ld %p step +\n",globalt-prevt,s);
    // prevt=globalt;
    break;
  };
  s->phase = p1;
}
