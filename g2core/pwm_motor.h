#ifndef PWM_MOTOR_H_ONCE
#define PWM_MOTOR_H_ONCE
#if PWM_MOTORS_AVAILABLE

# include "stepper.h"

# define PWM_MOTOR_COUNT 2

typedef struct pwm_motor {
  uint32_t x_counter; // total counter
  uint32_t counter;   // current counter
  Pio     *reg;       // PIO;
  uint32_t reg_mask;
} pwm_motor_t;

extern pwm_motor_t pwm_motors[PWM_MOTOR_COUNT];

void    setup_pwm_motors();
void    pwm_motors_step();

uint8_t nv_index_2_motor_index(const index_t index);
void    pwm_motor_print_out(nvObj_t *nv);
stat_t  pwm_motor_get_value(nvObj_t *nv);
stat_t  pwm_motor_set_value(nvObj_t *nv);

#endif // PWM_MOTORS_AVAILABLE
#endif // PWM_MOTOR_H_ONCE
