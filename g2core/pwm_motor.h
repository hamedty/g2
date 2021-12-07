#ifndef PWM_MOTOR_H_ONCE
#define PWM_MOTOR_H_ONCE

#include "g2core.h"
#include "config.h"
#include "encoder.h"
#include "canonical_machine.h"  // needed for cm_panic() in assertions
# include "stepper.h"

#if PWM_MOTORS_AVAILABLE

#if PWM_MOTORS_ARRANGEMENT == 1
# define PWM_MOTOR_COUNT 9
#else
# define PWM_MOTOR_COUNT 1
#endif

typedef struct pwm_motor {
  uint32_t x_counter_on; // total counter on
  uint32_t x_counter_off; // total counter off
  uint32_t counter;   // current counter
  Pio     *reg;       // PIO;
  uint32_t reg_mask;
  bool blockable;
} pwm_motor_t;


typedef struct sensor_blocking_data {
  int32_t blocked_counter;
  bool blocked_out;
} sensor_blocking_data_t;

extern pwm_motor_t pwm_motors[PWM_MOTOR_COUNT];
extern sensor_blocking_data_t sensor_blocking_data;

void    setup_pwm_motors();
void    pwm_motors_step();

uint8_t nv_index_2_motor_index(const index_t index);
void    pwm_motor_print_out(nvObj_t *nv);
stat_t  pwm_motor_get_value(nvObj_t *nv);
stat_t  pwm_motor_set_value(nvObj_t *nv);
stat_t pwm_motor_set_value_simple(uint8_t, uint32_t);

#endif // PWM_MOTORS_AVAILABLE
#endif // PWM_MOTOR_H_ONCE
