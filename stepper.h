#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>

typedef struct STEPPER {
  int32_t pos;
  uint8_t phase;
} STEPPER;

void stepper_init(STEPPER *s);
void stepper_update(STEPPER *s, uint8_t pattern);
void stepper_update_rev(STEPPER *s, uint8_t pattern);

#endif
