#if PWM_MOTORS_AVAILABLE

#include "pwm_motor.h"
#include "gpio.h"
#define BLOCKED_COUNTER_SMALL 40000
#define BLOCKED_COUNTER_BIG   200000

pwm_motor_t pwm_motors[PWM_MOTOR_COUNT] = {
  // { 0, 0, 0, PIOC, 1 << 17, 0, 0, 0 }, // M1 - Step 4 - PC17 - D46
  { 0, 1000, 0, PIOA, 1 << 0, 1, 0, 0 }, // M1 - V16 - PA0 - D69
  { 0, 0, 0, PIOC, 1 << 19, 0, 0, 0 }, // M2 - Step 5 - PC19 - D44
  { 0, 0, 0, PIOA, 1 << 19, 0, 0, 0 }, // M3 - Step 6 - PA19 - D42
  { 0, 0, 0, PIOC, 1 << 8, 1, 0, 0  }, // M4 - Step 7 - PC8 - D40
  { 0, 0, 0, PIOC, 1 << 6, 0, 0, 0  }, // M5 - Step 8 - PC6 - D38
  { 0, 0, 0, PIOC, 1 << 4, 0, 0, 0  }, // M6 - Step 9 - PC4 - D36
  // { 0, 0, 0, PIOC, 1 << 2, 1, 0, 0  }, // M7 - Step 10 - PC2 - D34
  { 0, 0, 0, PIOC, 1 << 2, 0, 0, 0  }, // M7 - Step 10 - PC2 - D34
  { 0, 0, 0, PIOD, 1 << 10, 0, 0, 0 }, // M8 - Step 11 - PD10 - D32
  { 0, 0, 0, PIOD, 1 << 9, 0, 0, 0  }, // M9 - Step 12 - PD9 - D30

};

void setup_pwm_motors() {
  // setup motors
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];
    m->reg->PIO_PER  = m->reg_mask; // Enable PIO
    m->reg->PIO_OER  = m->reg_mask; // Set to OUTPUT
    m->reg->PIO_PUDR = m->reg_mask; // Disable the pull up resistor
  }
}

void pwm_motors_step() {
  for (int i = 0; i < PWM_MOTOR_COUNT; i++) {
    pwm_motor *m = &pwm_motors[i];
    if (!m->x_counter_on) continue;
    if (m->blocked_out == 0) {

      if (m->counter == 0) {
        // step
        if (m->reg->PIO_ODSR & m->reg_mask) {
          m->reg->PIO_CODR = m->reg_mask; // clear
          m->counter = m->x_counter_off;
        }
        else {
          m->reg->PIO_SODR = m->reg_mask; // set
          m->counter = m->x_counter_on;
        }
      }
      m->counter--;

    }

    if (m->blocked_counter == 0) continue;

    // hysthersis calcs
    if (m->blocked_out) {
      if (m->blocked_in) {
        m->blocked_counter = BLOCKED_COUNTER_BIG;
      }
      else {
        m->blocked_counter--;
        if (m->blocked_counter <= 1) {
          m->blocked_out = 0;
          m->blocked_counter = 1;
        }
      }
    }
    else {
      if (m->blocked_in) {
        m->blocked_counter++;
        if (m->blocked_counter >= BLOCKED_COUNTER_SMALL) {
          m->blocked_out = 1;
          m->reg->PIO_CODR = m->reg_mask;
          m->blocked_counter = BLOCKED_COUNTER_BIG;
        }

      }
      else {
        m->blocked_counter = 1;
      }
    }
  }
}

uint8_t nv_index_2_motor_index(const index_t index)
{
  const char *ptr = cfgArray[index].token;

  ptr++;
  return atoi(ptr) - 1;
}

void pwm_motor_print_out(nvObj_t *nv) {
  // sprintf(cs.out_buf, fmt_gpio_out, nv->token, (int)nv->value_int);
  // xio_writeline(cs.out_buf);
}

stat_t pwm_motor_get_value(nvObj_t *nv)
{
  nv->value_flt = 10;
  return STAT_OK;
}

stat_t pwm_motor_set_value(nvObj_t *nv)
{
  uint8_t motor_index = nv_index_2_motor_index(nv->index);

  if (motor_index >= PWM_MOTOR_COUNT) return STAT_OK;

  pwm_motor *m = &pwm_motors[motor_index];
  m->x_counter_on = (uint32_t)nv->value_flt;
  if(motor_index != 0) { // everything other than 1st motor
    m->x_counter_off = m->x_counter_on;
  }
  if (m->x_counter_on == 0) {
    m->reg->PIO_CODR = m->reg_mask;
  }
  // if ((motor_index == (4-1)) || (motor_index == (7-1))) { // M4 & M7
  // if (motor_index == (4-1)) { // M4
  if ((motor_index == (4-1)) || (motor_index == (1-1))) { // M4 & M1
    if(d_in[4-1].state) {
      m->blocked_in = 1;
      m->blocked_out = 1;
      m->reg->PIO_CODR = m->reg_mask;
      m->blocked_counter = BLOCKED_COUNTER_BIG;
    }
  }
  return STAT_OK;
}

#endif // PWM_MOTORS_AVAILABLE
