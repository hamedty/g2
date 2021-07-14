#ifndef PWM_MOTOR_H_ONCE
#define PWM_MOTOR_H_ONCE

#include "stepper.h"

#define PWM_MOTOR_COUNT 1

typedef struct pwm_motor{
  uint32_t x_counter; // total counter
  uint32_t counter; // total counter
  uint32_t reg; // PIO;
  uint32_t reg_mask;
} pwm_motor_t;

pwm_motor_t pwm_motors [PWM_MOTOR_COUNT] = {
  {1000, 0, PIOA, 12},
};

void setup_pwm_motors() {
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor* m = &pwm_motors[i];
    m->reg->PIO_PER = m->reg_mask;  //Enable PIO
    m->reg->PIO_OER = m->reg_mask;  //Set to OUTPUT
    m->reg->PIO_PUDR = m->reg_mask; //Disable the pull up resistor
  }
}

#endif
